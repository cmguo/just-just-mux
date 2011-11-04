// RtpEsAudioTransfer.h

#ifndef   _PPBOX_MUX_RTP_ES_AUDIO_TRANSFER_H_
#define   _PPBOX_MUX_RTP_ES_AUDIO_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"
#include "ppbox/mux/Muxer.h"

namespace ppbox
{
    namespace mux
    {
        class RtpEsAudioTransfer
            : public RtpTransfer
        {
        public:
            RtpEsAudioTransfer(
                Muxer & muxer,
                boost::uint8_t type);

            ~RtpEsAudioTransfer();

            virtual void transfer(ppbox::demux::Sample & sample);

            virtual void get_rtp_info(ppbox::demux::MediaInfo & info);

            virtual void on_seek(boost::uint32_t time, boost::uint32_t play_time);

        private:
            boost::uint32_t time_scale_;
            boost::uint32_t index_;
            boost::uint8_t au_header_section_[4];
            std::vector<boost::uint8_t> packat_header_;

        };
    }
}
#endif // _PPBOX_MUX_RTP_ES_AUDIO_TRANSFER_H_
