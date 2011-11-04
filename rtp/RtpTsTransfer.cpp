// RtpTsTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpTsTransfer.h"
#include "ppbox/mux/rtp/RtpPacket.h"
#include "ppbox/mux/ts/Mpeg2Ts.h"

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

        void RtpTsTransfer::transfer(ppbox::demux::Sample & sample)
        {
            RtpTransfer::clear();
            std::vector<TsPacket> const & ts_buffers = 
                *(std::vector<TsPacket> const *)sample.context;
            for (boost::uint32_t i = 0; i < ts_buffers.size()-1; ++i) {
                RtpPacket p(sample.pts, false);
                p.push_buffers(ts_buffers[i].buffers);
                push_packet(p);
            }
            if (ts_buffers.size() > 0) {
                RtpPacket p(sample.pts, true);
                p.push_buffers(ts_buffers[ts_buffers.size()-1].buffers);
                push_packet(p);
                sample.context = (void*)&rtp_packets();
            } else {
                sample.context = NULL;
            }
        }


        void RtpTsTransfer::get_rtp_info(ppbox::demux::MediaInfo & info)
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

        void RtpTsTransfer::header_rtp_packet(ppbox::demux::Sample & tag)
        {
            RtpTransfer::clear();
            RtpPacket p(rep_info().timestamp, true);
            p.push_buffers(tag.data);
            push_packet(p);
            tag.context = (void*)&rtp_packets();
        }

    }
}
