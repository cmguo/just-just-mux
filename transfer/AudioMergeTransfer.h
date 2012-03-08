// AudioMergeTransfer.h

#ifndef   _PPBOX_MUX_AUDIO_MERGE_TRANSFER_H_
#define   _PPBOX_MUX_AUDIO_MERGE_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"

const boost::uint32_t AUDIO_MAX_DATA_SIZE = 1024 * 10;

namespace ppbox
{
    namespace mux
    {
        class AudioMergeTransfer
            : public Transfer
        {
        public:
            AudioMergeTransfer();

           ~AudioMergeTransfer();

            virtual void transfer(ppbox::demux::Sample & sample);

        private:
            boost::uint64_t last_audio_dts_;
            boost::uint8_t audio_buffer_[AUDIO_MAX_DATA_SIZE];
            boost::uint32_t audio_data_size_;

        };
    }
}
#endif // _PPBOX_MUX_TS_AUDIO_TRANSFER_H_
