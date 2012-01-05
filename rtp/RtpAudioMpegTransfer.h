// RtpAudioMpegTransfer.h

#ifndef _PPBOX_MUX_RTP_AUDIO_MPEG_TRANSFER_H_
#define _PPBOX_MUX_RTP_AUDIO_MPEG_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"
#include "ppbox/mux/Muxer.h"

namespace ppbox
{
    namespace mux
    {
        class RtpAudioMpegTransfer
            : public RtpTransfer
        {
        public:
            RtpAudioMpegTransfer(
                Muxer & muxer,
                boost::uint8_t type);

            ~RtpAudioMpegTransfer();

            virtual void transfer(ppbox::demux::Sample & sample);

            virtual void get_rtp_info(MediaInfoEx & info);

            virtual void on_seek(boost::uint32_t time, boost::uint32_t play_time);

        private:
            boost::uint16_t header_[2];
        };
    }
}
#endif // _PPBOX_MUX_RTP_AUDIO_MPEG_TRANSFER_H_
