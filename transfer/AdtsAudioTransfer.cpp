// AudioMergeTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/AdtsAudioTransfer.h"

namespace ppbox
{
    namespace mux
    {

        AdtsAudioTransfer::AdtsAudioTransfer()
        {
        }

        AdtsAudioTransfer::~AdtsAudioTransfer()
        {
        }

        void AdtsAudioTransfer::transfer(
            StreamInfo & info)
        {
            aac_config_.from_data(info.format_data);
        }

        void AdtsAudioTransfer::transfer(
            Sample & sample)
        {
            //std::cout << "Audio sample: dts = " << sample.dts << std::endl;
            StreamInfo const * audio_stream_info = (StreamInfo const *)sample.stream_info;
            unsigned int sampling_frequency_index = 0;
            unsigned int channel_configuration = 0;
            boost::uint32_t frame_size = sample.size;
            aac_config_.to_adts_data(sample.size, adts_header_);
            sample.data.push_front(boost::asio::buffer(adts_header_));
            sample.size += adts_header_.size();
        }

    } // namespace mux
} // namespace ppbox
