// RtpH264Transfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/rtp/RtpH264Transfer.h"

#include <ppbox/avformat/codec/avc/AvcCodec.h>
#include <ppbox/avformat/codec/avc/AvcConfig.h>
#include <ppbox/avformat/codec/avc/AvcNaluBuffer.h>
using namespace ppbox::avformat;

#include <framework/string/Base16.h>
#include <framework/string/Base64.h>

namespace ppbox
{
    namespace mux
    {

        static boost::uint32_t const TIME_SCALE = 90000;

        RtpH264Transfer::RtpH264Transfer()
            : RtpTransfer("RtpH264", 96, TIME_SCALE)
            , mtu_size_(1436)
            , sample_description_index_(boost::uint32_t(-1))
            , use_dts_(false)
        {
        }

        RtpH264Transfer::~RtpH264Transfer()
        {
        }

        void RtpH264Transfer::config(
            framework::configure::Config & conf)
        {
            RtpTransfer::config(conf);
            conf.register_module("RtpH264")
                << CONFIG_PARAM_NAME_RDWR("usedts", use_dts_);
        }

        void RtpH264Transfer::transfer(
            StreamInfo & info)
        {
            RtpTransfer::transfer(info); // call TimeScaleTransfer::transfer

            using namespace framework::string;
            std::string map_id_str = format(rtp_head_.mpt);

            AvcCodec & codec = *(AvcCodec *)info.codec.get();
            std::vector<boost::uint8_t> sps_data = codec.config().sequenceParameterSetNALUnit[0];
            std::vector<boost::uint8_t> pps_data = codec.config().pictureParameterSetNALUnit[0];

            boost::uint8_t const * profile_level_id = &sps_data.front() + 1;

            std::string profile_level_id_str = 
                Base16::encode(std::string((char const *)profile_level_id, 3));
            std::string sps = Base64::encode(&sps_data.front(), sps_data.size());
            std::string pps = Base64::encode(&pps_data.front(), pps_data.size());

            rtp_info_.sdp = "m=video 0 RTP/AVP " + map_id_str + "\r\n";
            rtp_info_.sdp += "a=rtpmap:" + map_id_str + " H264/" + format(TIME_SCALE) + "\r\n";
            rtp_info_.sdp += "a=framesize:" + map_id_str + " " + format(info.video_format.width)
                + "-" + format(info.video_format.height) + "\r\n";
            rtp_info_.sdp += "a=cliprect:0,0," 
                + format(info.video_format.height) + "," + format(info.video_format.width) + "\r\n";
            rtp_info_.sdp += "a=fmtp:" + map_id_str 
                + " packetization-mode=1" 
                + ";profile-level-id=" + profile_level_id_str
                + ";sprop-parameter-sets=" + sps + "," + pps + "\r\n";
            rtp_info_.sdp += "a=control:track" + format(info.index) + "\r\n";

            rtp_info_.stream_index = info.index;
        }

        void RtpH264Transfer::transfer(
            Sample & sample)
        {
            RtpTransfer::transfer(sample); // call TimeScaleTransfer::transfer

            boost::uint64_t rtp_time = use_dts_ ? sample.dts : sample.dts + sample.cts_delta;

            RtpTransfer::begin(sample);

            // add two sps pps rtp packet
            if (0 != sample_description_index_) {
                StreamInfo const & media = *(StreamInfo const *)sample.stream_info;

                sample_description_index_ = 0;
                AvcConfig const & avc_config = ((AvcCodec const *)media.codec.get())->config();

                for (size_t i = 0; i < avc_config.sequenceParameterSetNALUnit.size(); ++i) {
                    begin_packet(false, rtp_time, avc_config.sequenceParameterSetNALUnit[i].size());
                    push_buffers(boost::asio::buffer(avc_config.sequenceParameterSetNALUnit[i]));
                    finish_packet();
                }
                
                for (size_t i = 0; i < avc_config.pictureParameterSetNALUnit.size(); ++i) {
                    begin_packet(false, rtp_time, avc_config.pictureParameterSetNALUnit[i].size());
                    push_buffers(boost::asio::buffer(avc_config.pictureParameterSetNALUnit[i]));
                    finish_packet();
                }
            }

            std::vector<NaluBuffer> & nalus = 
                *(std::vector<NaluBuffer> *)sample.context;
            for (size_t i = 0; i < nalus.size(); ++i) {
                NaluBuffer & nalu = nalus[i];
                size_t l = nalu.size;
                if (l > (mtu_size_)) {
                    boost::uint8_t b = nalu.begin.dereference_byte();
                    prefix_[0][0] = (b & 0xE0) | 28;
                    prefix_[0][1] = (b | 0x80) & 0x9F;
                    nalu.begin.increment_byte(nalu.end);
                    --l;
                    NaluBuffer::BuffersPosition pos = nalu.begin;
                    nalu.begin.increment_bytes(nalu.end, mtu_size_ - 2);
                    begin_packet(false, rtp_time, mtu_size_);
                    push_buffers(boost::asio::buffer(prefix_[0], 2));
                    push_buffers(NaluBuffer::range_buffers(pos, nalu.begin));
                    finish_packet();
                    l -= mtu_size_ - 2;
                    prefix_[1][0] = prefix_[0][0];
                    prefix_[1][1] = prefix_[0][1] & 0x7F;
                    while (l > mtu_size_ - 2) {
                        NaluBuffer::BuffersPosition pos1 = nalu.begin;
                        nalu.begin.increment_bytes(nalu.end, mtu_size_ - 2);
                        begin_packet(false, rtp_time, mtu_size_);
                        push_buffers(boost::asio::buffer(prefix_[1], 2));
                        push_buffers(NaluBuffer::range_buffers(pos1, nalu.begin));
                        finish_packet();
                        l -= mtu_size_ - 2;
                    };
                    prefix_[2][0] = prefix_[1][0];
                    prefix_[2][1] = prefix_[1][1] | 0x40;
                    begin_packet(i == nalus.size() - 1, rtp_time, l + 2);
                    push_buffers(boost::asio::buffer(prefix_[2], 2));
                    push_buffers(nalu.buffers());
                    finish_packet();
                } else {
                    begin_packet(i == nalus.size() - 1, rtp_time, l);
                    push_buffers(nalu.buffers());
                    finish_packet();
                }
            }
            RtpTransfer::finish(sample);
        }

    } // namespace mux
} // namespace ppbox
