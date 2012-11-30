// AdtsAudioTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_ADTS_AUDIO_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_ADTS_AUDIO_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avformat/codec/aac/AacConfigHelper.h>

namespace ppbox
{
    namespace mux
    {

        class MpegAudioAdtsEncodeTransfer
            : public Transfer
        {
        public:
            MpegAudioAdtsEncodeTransfer();

            ~MpegAudioAdtsEncodeTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            std::vector<boost::uint8_t> adts_header_;
            ppbox::avformat::AacConfigHelper aac_config_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_ADTS_AUDIO_TRANSFER_H_
