// RtpEsVideoTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/rtp/RtpEsVideoTransfer.h"
#include "ppbox/mux/detail/BitsReader.h" // for Nalu

#include <ppbox/avformat/codec/avc/AvcCodec.h>
using namespace ppbox::avformat;

#include <util/buffers/BufferCopy.h>

#include <framework/string/Base16.h>
#include <framework/string/Base64.h>

namespace ppbox
{
    namespace mux
    {

        RtpEsVideoTransfer::RtpEsVideoTransfer(
            MuxerBase & muxer)
            : RtpTransfer(muxer, "RtpESVideo", 96)
            , mtu_size_(1436)
            , sample_description_index_(boost::uint32_t(-1))
            , use_dts_(0)
        {
            muxer.config().register_module("RtpESVideo")
                << CONFIG_PARAM_NAME_RDWR("usedts", use_dts_);
        }

        RtpEsVideoTransfer::~RtpEsVideoTransfer()
        {
        }

        void RtpEsVideoTransfer::transfer(
            StreamInfo & media)
        {
            using namespace framework::string;
            std::string map_id_str = format(rtp_head_.mpt);

            boost::uint8_t const * p = &media.format_data.at(0) + 5;
            boost::uint8_t const * sps_buf = p;
            boost::uint8_t const * pps_buf = p;
            boost::uint16_t sps_len = 0;
            boost::uint16_t pps_len = 0;
            size_t n = (*p++) & 31;
            for (size_t i = 0; i < n; ++i) {
                size_t l = (*p++);
                l = (l << 8) + (*p++);
                sps_buf = p;
                sps_len = l;
                p += l;
            }
            n = (*p++);
            for (size_t i = 0; i < n; ++i) {
                size_t l = (*p++);
                l = (l << 8) + (*p++);
                pps_buf = p;
                pps_len = l;
                p += l;
            }

            boost::uint8_t profile_level_id[3] = {
                sps_buf[1], 
                sps_buf[2], 
                sps_buf[3]
            };

            std::string profile_level_id_str = 
                Base16::encode(std::string((char *)profile_level_id, 3));
            std::string sps = Base64::encode(std::string((char *)sps_buf, sps_len));
            std::string pps = Base64::encode(std::string((char *)pps_buf, pps_len));

            rtp_info_.sdp = "m=video 0 RTP/AVP " + map_id_str + "\r\n";
            rtp_info_.sdp += "a=rtpmap:" + map_id_str + " H264/90000\r\n";
            rtp_info_.sdp += "a=framesize:" + map_id_str + " " + format(media.video_format.width)
                + "-" + format(media.video_format.height) + "\r\n";
            rtp_info_.sdp += "a=cliprect:0,0,"+format(media.video_format.height)+","+format(media.video_format.width)+ "\r\n";
            rtp_info_.sdp += "a=fmtp:" + map_id_str 
                + " packetization-mode=1" 
                + ";profile-level-id=" + profile_level_id_str
                + ";sprop-parameter-sets=" + sps + "," + pps + "\r\n";
            rtp_info_.sdp += "a=control:track" + format(media.index) + "\r\n";

            rtp_info_.stream_index = media.index;

            scale_.reset(media.time_scale, 90000);
        }

        void RtpEsVideoTransfer::transfer(
            Sample & sample)
        {
            StreamInfo const & media = *(StreamInfo const *)sample.stream_info;
            NaluList & nalus = *(NaluList *)sample.context;

            boost::uint64_t cts_time = 0;
            if (use_dts_) {
                cts_time = scale_.transfer(sample.dts);
                assert(cts_time == sample.dts * 90000 / media.time_scale);
            } else {
                cts_time = scale_.transfer(sample.dts + sample.cts_delta);
            }
            
            //std::cout << "video dts = " << sample.dts << std::endl;
            //std::cout << "video cts_delta = " << sample.cts_delta << std::endl;
            //std::cout << "video cts = " << cts_time << std::endl;

            RtpTransfer::begin(sample);

            MyBuffersLimit limit(sample.data.begin(), sample.data.end());
            // add two sps pps rtp packet
            if (sample.idesc != sample_description_index_) {
                sample_description_index_ = sample.idesc;
                AvcConfig const & avc_config = ((AvcCodec const *)media.codec)->config();

                for (size_t i = 0; i < avc_config.sequenceParameterSetNALUnit.size(); ++i) {
                    RtpPacket sps_p(cts_time, false);
                    sps_p.size = avc_config.sequenceParameterSetNALUnit[i].size();
                    sps_p.push_buffers(boost::asio::buffer(avc_config.sequenceParameterSetNALUnit[i]));
                    push_packet(sps_p);
                }
                
                for (size_t i = 0; i < avc_config.pictureParameterSetNALUnit.size(); ++i) {
                    RtpPacket pps_p(cts_time, false);
                    pps_p.size = avc_config.pictureParameterSetNALUnit[i].size();
                    pps_p.push_buffers(boost::asio::buffer(avc_config.pictureParameterSetNALUnit[i]));
                    push_packet(pps_p);
                }
            }

            for (size_t i = 0; i < nalus.size(); ++i) {
                size_t l = nalus[i].size;
                if (l > (mtu_size_)) {
                    boost::uint8_t b = nalus[i].begin.dereference_byte();
                    prefix_[0][0] = (b & 0xE0) | 28;
                    prefix_[0][1] = (b | 0x80) & 0x9F;
                    nalus[i].begin.increment_byte(limit);
                    --l;
                    RtpPacket p(cts_time, false);
                    MyBuffersPosition pos = nalus[i].begin;
                    nalus[i].begin.increment_bytes(limit, mtu_size_ - 2);
                    p.push_buffers(boost::asio::buffer(prefix_[0], 2));
                    p.push_buffers(MyBufferIterator(limit, pos, nalus[i].begin), MyBufferIterator());
                    p.size = mtu_size_;
                    push_packet(p);
                    l -= mtu_size_ - 2;
                    prefix_[1][0] = prefix_[0][0];
                    prefix_[1][1] = prefix_[0][1] & 0x7F;
                    while (l > mtu_size_ - 2) {
                        RtpPacket p(cts_time, false);
                        //p.size = 1438;
                        MyBuffersPosition pos1 = nalus[i].begin;
                        nalus[i].begin.increment_bytes(limit, mtu_size_ - 2);
                        p.push_buffers(boost::asio::buffer(prefix_[1], 2));
                        p.push_buffers(MyBufferIterator(limit, pos1, nalus[i].begin), MyBufferIterator());
                        p.size = mtu_size_;
                        push_packet(p);
                        l -= mtu_size_ - 2;
                    };
                    prefix_[2][0] = prefix_[1][0];
                    prefix_[2][1] = prefix_[1][1] | 0x40;
                    RtpPacket p2(cts_time, i == nalus.size() - 1);
                    p2.size = l + 2;
                    p2.push_buffers(boost::asio::buffer(prefix_[2], 2));
                    p2.push_buffers(
                        MyBufferIterator(limit, nalus[i].begin, nalus[i].end), MyBufferIterator());
                    push_packet(p2);
                } else {
                    RtpPacket p(cts_time, i == nalus.size() - 1);
                    p.size = l;
                    p.push_buffers(MyBufferIterator(limit, nalus[i].begin, nalus[i].end), MyBufferIterator());
                    push_packet(p);
                }
            }
            RtpTransfer::finish(sample);
        }

    } // namespace mux
} // namespace ppbox
