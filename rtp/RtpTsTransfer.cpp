// RtpTsTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/TsTransfer.h"
#include "ppbox/mux/rtp/RtpTsTransfer.h"
#include "ppbox/mux/rtp/RtpPacket.h"

const boost::uint32_t TS_PACKETS_PER_RTP_PACKET       = 7;

namespace ppbox
{
    namespace mux
    {

        RtpTsTransfer::RtpTsTransfer(
            Muxer & muxer,
            boost::uint8_t type)
            : RtpTransfer(type)
        {
            muxer.Config().register_module("RtpTs")
                << CONFIG_PARAM_NAME_RDWR("sequence", rtp_head_.sequence)
                << CONFIG_PARAM_NAME_RDWR("timestamp", rtp_head_.timestamp)
                << CONFIG_PARAM_NAME_RDWR("ssrc", rtp_head_.ssrc);
        }

        RtpTsTransfer::~RtpTsTransfer()
        {
        }

        void RtpTsTransfer::transfer(
            ppbox::demux::Sample & sample)
        {
            RtpTransfer::clear(sample.ustime + sample.cts_delta * 1000000 / sample.media_info->time_scale);
            std::vector<size_t> const & off_segs = 
                *(std::vector<size_t> const *)sample.context;
            std::deque<boost::asio::const_buffer>::const_iterator buf_beg = sample.data.begin();
            std::deque<boost::asio::const_buffer>::const_iterator buf_end = sample.data.end();
            boost::uint32_t i = TS_PACKETS_PER_RTP_PACKET;
            for (; i + 1 < off_segs.size(); i += TS_PACKETS_PER_RTP_PACKET) {
                // i + 1，这里+1是为了保证至少有一个RTP在后面生成，因为需要mark置为true
                buf_end = sample.data.begin() + off_segs[i];
                RtpPacket p(sample.dts + sample.cts_delta, false);
                p.size = TS_PACKETS_PER_RTP_PACKET * AP4_MPEG2TS_PACKET_SIZE;
                p.push_buffers(buf_beg, buf_end);
                push_packet(p);
                buf_beg = buf_end;
            }
            i -= TS_PACKETS_PER_RTP_PACKET;
            buf_end = sample.data.end();
            RtpPacket p(sample.dts + sample.cts_delta, true);
            p.size = (off_segs.size() - i) * AP4_MPEG2TS_PACKET_SIZE;
            p.push_buffers(buf_beg, buf_end);
            push_packet(p);

            sample.context = (void*)&rtp_packets();
        }


        void RtpTsTransfer::get_rtp_info(MediaInfoEx & info)
        {
            rep_info().sdp   = "m=video 0 RTP/AVP 33\r\n";
            rep_info().sdp += "a=rtpmap:33 MP2T/90000\r\n";
            rep_info().sdp += "a=cliprect:0,0,"
                + framework::string::format(info.video_format.height)+","
                +framework::string::format(info.video_format.width)+"\r\n";
            rep_info().sdp += "a=framesize: 33 " 
                + framework::string::format(info.video_format.width)+ "-"
                + framework::string::format(info.video_format.height) + "\r\n";
            rep_info().sdp += "a=control:index=-1\r\n";

            rep_info().stream_index = 0;
            rep_info().timestamp = rtp_head_.timestamp;
            rep_info().seek_time = 0;
            rep_info().ssrc = rtp_head_.ssrc;
            rep_info().sequence = rtp_head_.sequence;

            info.attachment = (void*)&rep_info();
        }

        void RtpTsTransfer::on_seek(boost::uint32_t time, boost::uint32_t play_time)
        {
            boost::uint32_t time_offset = time * 90;
            boost::uint32_t play_time_offset = (play_time) * 90;
            rtp_head_.timestamp += play_time_offset;
            rep_info().timestamp = rtp_head_.timestamp + time_offset;
            rep_info().sequence = rtp_head_.sequence;
            rep_info().seek_time = time;
        }

        void RtpTsTransfer::header_rtp_packet(
            ppbox::demux::Sample & tag)
        {
            RtpTransfer::clear(0);
            RtpPacket p(rep_info().timestamp, true);
            p.push_buffers(tag.data);
            push_packet(p);
            tag.context = (void*)&rtp_packets();
        }

    }
}
