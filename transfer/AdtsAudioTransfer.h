// AdtsAudioTransfer.h

#ifndef   _PPBOX_MUX_ADTS_AUDIO_TRANSFER_H_
#define   _PPBOX_MUX_ADTS_AUDIO_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"

namespace ppbox
{
    namespace mux
    {

        class AdtsAudioTransfer
            : public Transfer
        {
        public:
            AdtsAudioTransfer();

            ~AdtsAudioTransfer();

            virtual void transfer(
                ppbox::demux::Sample & sample);

        private:
            void MakeAdtsHeaderWithData(
                boost::uint8_t bits[7], 
                boost::uint32_t frame_size,
                boost::uint32_t sampling_frequency_index,
                boost::uint32_t channel_configuration);

            void MakeAdtsHeaderWithBuffer(
                boost::uint8_t bits[7], 
                boost::uint32_t frame_size,
                boost::uint8_t const * extra,
                boost::uint32_t extraLen);

        private:
            boost::uint8_t adts_header_[7];
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_ADTS_AUDIO_TRANSFER_H_
