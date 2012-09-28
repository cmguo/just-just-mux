// FlvAudioTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvAudioTransfer.h"

using namespace ppbox::avformat;

#include <framework/system/BytesOrder.h>
using namespace framework::system;

namespace ppbox
{
    namespace mux
    {

        void FlvAudioTransfer::transfer(Sample & sample)
        {
            boost::uint32_t data_length = sample.size;
            setTagSizeAndTimestamp(data_length+2, (boost::uint32_t)sample.time);
            sample.data.push_front(boost::asio::buffer((boost::uint8_t*)&audiotagheader_, 2));
            sample.data.push_front(tag_buffer());
            previous_tag_size_ = data_length + 13;
            previous_tag_size_ = BytesOrder::host_to_big_endian_long(previous_tag_size_);
            sample.data.push_back(boost::asio::buffer((boost::uint8_t*)&previous_tag_size_, 4));
            sample.size += 17;
        }

        void FlvAudioTransfer::transfer(MediaInfoEx & mediainfo)
        {
            switch(mediainfo.sub_type)
            {
            case AUDIO_TYPE_MP4A:
                audiotagheader_.SoundFormat = 10;
                break;
            case AUDIO_TYPE_MP1A:
                audiotagheader_.SoundFormat = 2;
                break;
            case AUDIO_TYPE_WMA2:
                audiotagheader_.SoundFormat = 11;
                break;
            default:
                audiotagheader_.SoundFormat = 10;
                break;
            }

            if (mediainfo.audio_format.sample_rate >= 44100 ) {
                audiotagheader_.SoundRate = 3;
            } else if (mediainfo.audio_format.sample_rate >= 22000 ){
                audiotagheader_.SoundRate = 2;
            } else if (mediainfo.audio_format.sample_rate >= 11000) {
                audiotagheader_.SoundRate = 1;
            } else if (mediainfo.audio_format.sample_rate >= 5500) {
                audiotagheader_.SoundRate = 0;
            } else {
                audiotagheader_.SoundRate = 0;
            }

            if (mediainfo.audio_format.channel_count <= 1) {
                // for aac always 1;
                audiotagheader_.SoundType = 1;
            } else {
                audiotagheader_.SoundType = 1;
            }

            if (8 == mediainfo.audio_format.sample_size) {
                audiotagheader_.SoundSize = 0;
            } else {
                audiotagheader_.SoundSize = 1;
            }
            audiotagheader_.AACPacketType = 1;
        }

    }
}
