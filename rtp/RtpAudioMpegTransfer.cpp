// RtpAudioMpegTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/rtp/RtpAudioMpegTransfer.h"

namespace ppbox
{
    namespace mux
    {

        RtpAudioMpegTransfer::RtpAudioMpegTransfer(
            MuxerBase & muxer)
            : RtpTransfer(muxer, "RtpAudioMpegTransfer", 97)
        {
            header_[0] = 0;
            header_[1] = 0;
        }

        RtpAudioMpegTransfer::~RtpAudioMpegTransfer()
        {
        }

        void RtpAudioMpegTransfer::transfer(
            StreamInfo & media)
        {
            using namespace framework::string;

            std::string map_id_str = format(rtp_head_.mpt);
            rtp_info_.sdp = "m=audio 0 RTP/AVP " + map_id_str + "\r\n";
            rtp_info_.sdp += "a=rtpmap:" + map_id_str + " mpa/90000" 
                + "/" + format(media.audio_format.channel_count)
                + "\r\n";
            rtp_info_.sdp += "a=control:track" + format(media.index) + "\r\n";

            rtp_info_.stream_index = media.index;

            scale_.reset(media.time_scale, 90000);
        }

        void RtpAudioMpegTransfer::transfer(
            Sample & sample)
        {
            RtpTransfer::begin(sample);
            RtpPacket packet(scale_.transfer(sample.dts), true);
            packet.size = sample.size + 4;
            packet.push_buffers(boost::asio::buffer(header_, 4));
            packet.push_buffers(sample.data);
            push_packet(packet);
            RtpTransfer::finish(sample);
        }

    } // namespace mux
} // namespace ppbox
