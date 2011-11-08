// RtpEsVideoTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpEsVideoTransfer.h"

#include <util/buffers/BufferCopy.h>

namespace ppbox
{
    namespace mux
    {

        RtpEsVideoTransfer::RtpEsVideoTransfer(
            Muxer & muxer,
            boost::uint8_t type)
            : RtpTransfer(type)
        {
            muxer.Config().register_module("RtpESVideo")
                << CONFIG_PARAM_NAME_RDWR("sequence", rtp_head_.sequence)
                << CONFIG_PARAM_NAME_RDWR("timestamp", rtp_head_.timestamp)
                << CONFIG_PARAM_NAME_RDWR("ssrc", rtp_head_.ssrc);
        }

        RtpEsVideoTransfer::~RtpEsVideoTransfer()
        {
        }

        void RtpEsVideoTransfer::transfer(ppbox::demux::Sample & sample)
        {
            RtpTransfer::clear();
            ppbox::demux::MediaInfo const * video_info = sample.media_info;
            NaluList & nalus = *(NaluList *)sample.context;

            boost::uint32_t cts_time = boost::uint32_t(
                ((boost::uint64_t)sample.dts + sample.cts_delta) * 90000 / video_info->time_scale);

            MyBuffersLimit limit(sample.data.begin(), sample.data.end());

            //boost::uint8_t nalu_len = (boost::uint8_t)(video_info->format_data[4] & 3) + 1;
            //boost::uint32_t buffer_length = util::buffers::buffer_size(sample.data);
            //boost::uint32_t len = buffer_length;
            for (size_t i = 0; i < nalus.size(); ++i) {
                size_t l = nalus[i].size;
                if (l > 1438) {
                    boost::uint8_t b = nalus[i].begin.dereference_byte();
                    prefix_[0][0] = (b & 0xE0) | 28;
                    prefix_[0][1] = (b | 0x80) & 0x9F;
                    nalus[i].begin.increment_byte(limit);
                    --l;
                    RtpPacket p(cts_time, false);
                    MyBuffersPosition pos = nalus[i].begin;
                    nalus[i].begin.increment_bytes(limit, 1436);
                    p.push_buffers(boost::asio::buffer(prefix_[0], 2));
                    p.push_buffers(MyBufferIterator(limit, pos, nalus[i].begin), MyBufferIterator());
                    push_packet(p);
                    l -= 1436;
                    prefix_[1][0] = prefix_[0][0];
                    prefix_[1][1] = prefix_[0][1] & 0x7F;
                    while (l > 1436) {
                        RtpPacket p(cts_time, false);
                        MyBuffersPosition pos1 = nalus[i].begin;
                        nalus[i].begin.increment_bytes(limit, 1436);
                        p.push_buffers(boost::asio::buffer(prefix_[1], 2));
                        p.push_buffers(MyBufferIterator(limit, pos1, nalus[i].begin), MyBufferIterator());
                        push_packet(p);
                        l -= 1436;
                    };
                    prefix_[2][0] = prefix_[1][0];
                    prefix_[2][1] = prefix_[1][1] | 0x40;
                    RtpPacket p2(cts_time, i == nalus.size() - 1);
                    p2.push_buffers(boost::asio::buffer(prefix_[2], 2));
                    p2.push_buffers(
                        MyBufferIterator(limit, nalus[i].begin, nalus[i].end), MyBufferIterator());
                    push_packet(p2);
                } else {
                    RtpPacket p(cts_time, i == nalus.size() - 1);
                    p.push_buffers(MyBufferIterator(limit, nalus[i].begin, nalus[i].end), MyBufferIterator());
                    push_packet(p);
                }
            }
            sample.context = (void*)&rtp_packets();
        }

        void RtpEsVideoTransfer::get_rtp_info(ppbox::demux::MediaInfo & info)
        {
            using namespace framework::string;
            std::string map_id_str = format(rtp_head_.mpt);

            boost::uint8_t const * p = &info.format_data.at(0) + 5;
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
                sps_buf[2] & 0xD0, 
                sps_buf[3]
            };

            std::string profile_level_id_str = 
                Base16::encode(std::string((char *)profile_level_id, 3));
            std::string sps = Base64::encode(std::string((char *)sps_buf, sps_len));
            std::string pps = Base64::encode(std::string((char *)pps_buf, pps_len));

            rep_info().sdp = "m=video 0 RTP/AVP " + map_id_str + "\r\n";
            rep_info().sdp += "a=rtpmap:" + map_id_str + " H264/90000\r\n";
            rep_info().sdp += "a=framesize:" + map_id_str + " " + format(info.video_format.width)
                + "-" + format(info.video_format.height) + "\r\n";
            rep_info().sdp += "a=fmtp:" + map_id_str 
                + " profile-level-id=" + profile_level_id_str
                + "; packetization-mode=1" 
                + "; sprop-parameter-sets=" + sps + "," + pps + "\r\n";
            rep_info().sdp += "a=control:index=" + format(info.index) + "\r\n";

            rep_info().stream_index = info.index;
            rep_info().timestamp = rtp_head_.timestamp;
            rep_info().seek_time = 0;
            rep_info().ssrc = rtp_head_.ssrc;
            rep_info().sequence = rtp_head_.sequence;

            info.attachment = (void*)&rep_info();
        }

        void RtpEsVideoTransfer::on_seek(boost::uint32_t time, boost::uint32_t play_time)
        {
            boost::uint32_t time_offset = time * 90;
            //boost::uint32_t play_time_offset = play_time * 90;
            boost::uint32_t play_time_offset = (play_time+10000) * 90;
            rtp_head_.timestamp += play_time_offset;
            rep_info().timestamp = rtp_head_.timestamp + time_offset;
            rep_info().sequence = rtp_head_.sequence;
            rep_info().seek_time = time;
        }
    }
}
