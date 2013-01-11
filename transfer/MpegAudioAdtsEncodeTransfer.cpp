// MpegAudioAdtsEncodeTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/MpegAudioAdtsEncodeTransfer.h"

#include <ppbox/avformat/codec/aac/AacCodec.h>
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        MpegAudioAdtsEncodeTransfer::MpegAudioAdtsEncodeTransfer()
        {
        }

        MpegAudioAdtsEncodeTransfer::~MpegAudioAdtsEncodeTransfer()
        {
        }

        void MpegAudioAdtsEncodeTransfer::transfer(
            StreamInfo & info)
        {
        }

        void MpegAudioAdtsEncodeTransfer::transfer(
            Sample & sample)
        {
            AacConfigHelper const & config_helper = 
                ((AacCodec *)sample.stream_info->codec.get())->config_helper();

            config_helper.to_adts_data(sample.size, adts_header_);
            sample.data.push_front(boost::asio::buffer(adts_header_));
            sample.size += adts_header_.size();
        }

    } // namespace mux
} // namespace ppbox
