// AudioMergeTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/AudioMergeTransfer.h"

#include <util/buffers/BufferCopy.h>

namespace ppbox
{
    namespace mux
    {

        AudioMergeTransfer::AudioMergeTransfer()
            : last_audio_dts_(-1)
            , audio_data_size_(0)
        {
        }

        AudioMergeTransfer::~AudioMergeTransfer()
        {
        }

        void AudioMergeTransfer::transfer(ppbox::demux::Sample & sample)
        {
            size_t audio_frame_size = sample.size;
            if (last_audio_dts_ == boost::uint64_t(-1)) {
                last_audio_dts_ = sample.dts;
            }
            if ((boost::int32_t)(sample.dts - last_audio_dts_) > 400
                || (audio_data_size_+audio_frame_size) > AUDIO_MAX_DATA_SIZE) {
                    if (audio_data_size_ != 0) {
                        sample.data.clear();
                        sample.data.push_back(boost::asio::buffer(audio_buffer_, audio_data_size_));
                        sample.size = audio_data_size_;
                        audio_data_size_ = 0;
                    }
            } else {
                if (audio_data_size_ == 0) {
                    last_audio_dts_ = sample.dts;
                }
                util::buffers::buffer_copy(
                    boost::asio::buffer(audio_buffer_, AUDIO_MAX_DATA_SIZE),
                    sample.data,
                    audio_frame_size,
                    audio_data_size_);
                audio_data_size_ += audio_frame_size;
                sample.size = 0;
                sample.data.clear();
            }
        }

    }
}
