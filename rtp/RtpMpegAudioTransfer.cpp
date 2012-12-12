// RtpMpegAudioTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/rtp/RtpMpegAudioTransfer.h"

namespace ppbox
{
    namespace mux
    {

        static boost::uint32_t const TIME_SCALE = 90000;

        RtpMpegAudioTransfer::RtpMpegAudioTransfer()
            : RtpTransfer("RtpAudioMpegTransfer", 97, TIME_SCALE)
        {
            header_[0] = 0;
            header_[1] = 0;
        }

        RtpMpegAudioTransfer::~RtpMpegAudioTransfer()
        {
        }

        void RtpMpegAudioTransfer::transfer(
            StreamInfo & info)
        {
            using namespace framework::string;

            RtpTransfer::transfer(info);

            std::string map_id_str = format(rtp_head_.mpt);
            rtp_info_.sdp = "m=audio 0 RTP/AVP " + map_id_str + "\r\n";
            rtp_info_.sdp += "a=rtpmap:" + map_id_str + " mpa/" + format(TIME_SCALE) 
                + "/" + format(info.audio_format.channel_count)
                + "\r\n";
            rtp_info_.sdp += "a=control:track" + format(info.index) + "\r\n";

            rtp_info_.stream_index = info.index;
        }

        void RtpMpegAudioTransfer::transfer(
            Sample & sample)
        {
            RtpTransfer::transfer(sample); // call TimeScaleTransfer::transfer

            RtpTransfer::begin(sample);

            begin_packet(true, sample.dts, sample.size);
            push_buffers(boost::asio::buffer(header_, 4));
            push_buffers(sample.data);
            finish_packet();

            RtpTransfer::finish(sample);
        }

    } // namespace mux
} // namespace ppbox
