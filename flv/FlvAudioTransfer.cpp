// FlvAudioTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvAudioTransfer.h"

#include <ppbox/avformat/codec/aac/AacCodec.h>
using namespace ppbox::avformat;

#include <framework/system/BytesOrder.h>
using namespace framework::system;

namespace ppbox
{
    namespace mux
    {

        FlvAudioTransfer::FlvAudioTransfer()
            : FlvTransfer(FlvTagType::AUDIO)
        {
        }

        void FlvAudioTransfer::transfer(
            StreamInfo & info)
        {
            switch(info.sub_type)
            {
            case AUDIO_TYPE_MP4A:
                header_.SoundFormat = 10;
                break;
            case AUDIO_TYPE_MP1A:
                header_.SoundFormat = 2;
                break;
            case AUDIO_TYPE_WMA2:
                header_.SoundFormat = 11;
                break;
            default:
                header_.SoundFormat = 10;
                break;
            }

            if (info.audio_format.sample_rate >= 44100 ) {
                header_.SoundRate = 3;
            } else if (info.audio_format.sample_rate >= 22000 ){
                header_.SoundRate = 2;
            } else if (info.audio_format.sample_rate >= 11000) {
                header_.SoundRate = 1;
            } else if (info.audio_format.sample_rate >= 5500) {
                header_.SoundRate = 0;
            } else {
                header_.SoundRate = 0;
            }

            if (info.audio_format.channel_count <= 1) {
                // for aac always 1;
                header_.SoundType = 1;
            } else {
                header_.SoundType = 1;
            }

            if (info.audio_format.sample_size == 8) {
                header_.SoundSize = 0;
            } else {
                header_.SoundSize = 1;
            }
            header_.AACPacketType = 1;
        }

        void FlvAudioTransfer::transfer(
            Sample & sample)
        {
            FormatBuffer buf(header_buffer_, sizeof(header_buffer_));
            FlvOArchive flv_archive(buf);
            flv_archive << header_;
            sample.data.push_front(buf.data());
            sample.size += buf.size();

            FlvTransfer::transfer(sample);
        }

        void FlvAudioTransfer::stream_header(
            StreamInfo const & info, 
            Sample & sample)
        {
            sample.stream_info = &info;
            if (info.sub_type == AUDIO_TYPE_MP4A) {
                header_.AACPacketType = 0;
                AacConfigHelper const & config = ((AacCodec *)info.codec)->config_helper();
                config.to_data(config_data_);
                sample.data.push_back(boost::asio::buffer(config_data_));
                sample.size += config_data_.size();
                transfer(sample);
                header_.AACPacketType = 1; // restore
            } else {
                assert(false);
            }
        }

    } // namespace mux
} // namespace ppbox
