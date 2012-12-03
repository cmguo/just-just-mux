// RtpMpegAudioTransfer.h

#ifndef _PPBOX_MUX_RTP_RTP_MPEG_AUDIO_TRANSFER_H_
#define _PPBOX_MUX_RTP_RTP_MPEG_AUDIO_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"

namespace ppbox
{
    namespace mux
    {

        class RtpMpegAudioTransfer
            : public RtpTransfer
        {
        public:
            RtpMpegAudioTransfer();

            ~RtpMpegAudioTransfer();

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

#endif // _PPBOX_MUX_RTP_RTP_MPEG_AUDIO_TRANSFER_H_
