// RtpAudioMpegTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/rtp/RtpAudioMpegTransfer.h"

namespace ppbox
{
    namespace mux
    {

        RtpAudioMpegTransfer::RtpAudioMpegTransfer(
            Muxer & muxer)
            : RtpTransfer(muxer, "RtpAudioMpegTransfer", 97)
        {
            header_[0] = 0;
            header_[1] = 0;
        }

        RtpAudioMpegTransfer::~RtpAudioMpegTransfer()
        {
        }

        void RtpAudioMpegTransfer::transfer(
            MediaInfoEx & info)
        {
            using namespace framework::string;

            std::string map_id_str = format(rtp_head_.mpt);
            rtp_info_.sdp = "m=audio 0 RTP/AVP " + map_id_str + "\r\n";
            rtp_info_.sdp += "a=rtpmap:" + map_id_str + " mpa/90000" 
                + "/" + format(info.audio_format.channel_count)
                + "\r\n";
            rtp_info_.sdp += "a=control:index=" + format(info.index) + "\r\n";

            rtp_info_.stream_index = info.index;

            time_scale_in_ms_ = 90;
        }

        void RtpAudioMpegTransfer::transfer(
            ppbox::demux::Sample & sample)
        {
            MediaInfoEx const * audio_info = (MediaInfoEx const *)sample.media_info;
            RtpTransfer::clear(sample.ustime);
            RtpPacket packet(
                (sample.dts + sample.cts_delta) * 90000 / audio_info->time_scale, 
                true);
            packet.size = sample.size + 4;
            packet.push_buffers(boost::asio::buffer(header_, 4));
            packet.push_buffers(sample.data);
            push_packet(packet);
            sample.context = (void*)&packets_;
        }

    }
}
