// RtpAudioMpegTransfer.h

#ifndef _PPBOX_MUX_RTP_RTP_AUDIO_MPEG_TRANSFER_H_
#define _PPBOX_MUX_RTP_RTP_AUDIO_MPEG_TRANSFER_H_

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
                MuxerBase & muxer);

            ~RtpAudioMpegTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            boost::uint16_t header_[2];
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_AUDIO_MPEG_TRANSFER_H_
