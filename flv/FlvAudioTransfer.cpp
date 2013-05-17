// FlvAudioTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvAudioTransfer.h"

#include <ppbox/avformat/flv/FlvFormat.h>
#include <ppbox/avformat/flv/FlvEnum.h>
using namespace ppbox::avformat;

#include <ppbox/avcodec/CodecType.h>
using namespace ppbox::avcodec;

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
            FlvFormat flv;
            CodecInfo const * codec = flv.codec_from_codec(info.type, info.sub_type);
            if (codec) {
                header_.SoundFormat = (boost::uint8_t)codec->format;
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

            if (info.audio_format.sample_size == 8) {
                header_.SoundSize = 0;
            } else {
                header_.SoundSize = 1;
            }

            if (info.audio_format.channel_count <= 1) {
                // for aac always 1;
                header_.SoundType = 0;
            } else {
                header_.SoundType = 1;
            }

            if (info.sub_type == AudioSubType::MP4A) {
                // for aac always 1;
                header_.SoundType = 1;
                // for aac always 3;
                header_.SoundRate = 3;
                header_.AACPacketType = 1;
            }
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
            if (info.sub_type == AudioSubType::MP4A) {
                header_.AACPacketType = 0;
                sample.data.push_back(boost::asio::buffer(info.format_data));
                sample.size += info.format_data.size();
                transfer(sample);
                header_.AACPacketType = 1; // restore
            }
        }

    } // namespace mux
} // namespace ppbox
