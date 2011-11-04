// FlvAudioTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvAudioTransfer.h"

#include <framework/system/BytesOrder.h>
#include <util/buffers/BufferSize.h>
using namespace framework::system;

namespace ppbox
{
    namespace mux
    {

        void FlvAudioTransfer::transfer(ppbox::demux::Sample & sample)
        {
            ppbox::demux::MediaInfo const * audio_stream_info = sample.media_info;
            flvtag_.TagType = TAG_TYPE_AUDIO;
            boost::uint8_t audio_attribute = 0xFF;
            switch(audio_stream_info->sub_type)
            {
            case ppbox::demux::AUDIO_TYPE_MP4A:
                audio_attribute = audio_attribute&0xAF;
                break;
            case ppbox::demux::AUDIO_TYPE_MP1A:
                audio_attribute = audio_attribute&0x2F;
                break;
            case ppbox::demux::AUDIO_TYPE_WMA2:
                audio_attribute = audio_attribute&0xDF;
                break;
            default:
                audio_attribute = audio_attribute&0xAF;
                break;
            }

            if (audio_stream_info->audio_format.sample_rate >= 44100 ) {
                audio_attribute = audio_attribute&0xFF;
            } else if (audio_stream_info->audio_format.sample_rate >= 24000 ){
                audio_attribute = audio_attribute&0xFB;
            } else if (audio_stream_info->audio_format.sample_rate >= 12000) {
                audio_attribute = audio_attribute&0xF7;
            } else if (audio_stream_info->audio_format.sample_rate >= 6000) {
                audio_attribute = audio_attribute&0xF3;
            } else {
                audio_attribute = audio_attribute&0xFF;
            }

            if (audio_stream_info->audio_format.channel_count <= 1) {
                // for aac always 1;
                audio_attribute = audio_attribute&0xFF;
            } else {
                audio_attribute = audio_attribute&0xFF;
            }

            if (8 == audio_stream_info->audio_format.sample_size) {
                audio_attribute = audio_attribute&0xFE;
            } else {
                audio_attribute = audio_attribute&0xFF;
            }
            AudioTagHeader audiotagheader;
            audiotagheader.SoundAttribute = audio_attribute;
            audiotagheader.AACPacketType = 0x01;
            boost::uint32_t data_length = sample.size;
            setTagSizeAndTimestamp(flvtag_, data_length+2, sample.time);
            sample_head_buffer_.resize(13);
            memcpy(&sample_head_buffer_.at(0), (boost::uint8_t*)&flvtag_, 11);
            memcpy(&sample_head_buffer_.at(0)+11, (boost::uint8_t*)&audiotagheader, 2);
            sample.data.push_front(boost::asio::buffer(&sample_head_buffer_.at(0), sample_head_buffer_.size()));
            previous_tag_size_ = data_length + sizeof(AudioTagHeader) + 11;
            previous_tag_size_ = BytesOrder::host_to_big_endian_long(previous_tag_size_);
            sample.data.push_back(boost::asio::buffer((boost::uint8_t*)&previous_tag_size_, 4));
            sample.size += 17;
        }

    }
}
