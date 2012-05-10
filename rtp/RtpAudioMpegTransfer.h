// RtpAudioMpegTransfer.h

#ifndef _PPBOX_MUX_RTP_AUDIO_MPEG_TRANSFER_H_
#define _PPBOX_MUX_RTP_AUDIO_MPEG_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"

namespace ppbox
{
    namespace mux
    {

        class RtpAudioMpegTransfer
            : public RtpTransfer
        {
        public:
            RtpAudioMpegTransfer(
                Muxer & muxer);

            ~RtpAudioMpegTransfer();

            virtual void transfer(
                MediaInfoEx & info);

            virtual void transfer(
                ppbox::demux::Sample & sample);

        private:
            boost::uint16_t header_[2];
        };
    }
}
#endif // _PPBOX_MUX_RTP_AUDIO_MPEG_TRANSFER_H_