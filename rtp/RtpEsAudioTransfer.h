// RtpEsAudioTransfer.h

#ifndef _PPBOX_MUX_RTP_RTP_ES_AUDIO_TRANSFER_H_
#define _PPBOX_MUX_RTP_RTP_ES_AUDIO_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"

namespace ppbox
{
    namespace mux
    {

        class RtpEsAudioTransfer
            : public RtpTransfer
        {
        public:
            RtpEsAudioTransfer(
                MuxerBase & muxer);

            ~RtpEsAudioTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

            virtual void on_seek(
                boost::uint32_t time);

        private:
            boost::uint8_t index_;
            boost::uint8_t time_adjust_;
            boost::uint8_t au_header_section_[4];
            std::vector<boost::uint8_t> packat_header_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_ES_AUDIO_TRANSFER_H_
