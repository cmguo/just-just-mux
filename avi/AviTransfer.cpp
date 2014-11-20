// AviTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/avi/AviTransfer.h"
#include "ppbox/mux/avi/AviDataContext.h"

#include <ppbox/avformat/avi/AviFormat.h>
using namespace ppbox::avformat;
#include <ppbox/avbase/object/Object.hpp>
using namespace ppbox::avbase;

#include <framework/system/BytesOrder.h>
using namespace framework::system;

namespace ppbox
{
    namespace mux
    {

        AviTransfer::AviTransfer(
            AviStream * stream, 
            AviDataContext * ctx)
            : stream_(stream)
            , ctx_(ctx)
        {
            buffer_[0] = 0;
            buffer_[1] = 0;
        }

        AviTransfer::~AviTransfer()
        {
        }

        void AviTransfer::transfer(
            StreamInfo & info)
        {
            AviFormat avi;
            boost::system::error_code ec;
            if (!avi.finish_from_codec(info, ec)) {
                return;
            }

            stream_->timescale(info.time_scale);
            if (info.type == StreamType::VIDE) {
                stream_->sample_duration(info.time_scale * info.video_format.frame_rate_den / info.video_format.frame_rate_num);
                stream_->handler(info.sub_type);
                AviStreamFormatBox::VideoFormat & video(stream_->video());
                video.biWidth = info.video_format.width;
                video.biHeight = info.video_format.height;
                video.biCompression = info.sub_type;
            } else {
                stream_->sample_duration(info.time_scale * info.audio_format.sample_per_frame / info.audio_format.sample_rate);
                AviStreamFormatBox::AudioFormat & audio(stream_->audio());
                audio.wFormatTag = info.sub_type;
                audio.nChannels = info.audio_format.channel_count;
                audio.nSamplesPerSec = info.audio_format.sample_rate;
                audio.nBlockAlign = info.audio_format.sample_per_frame;
                audio.cbSize = info.format_data.size();
                audio.cbData = info.format_data;
            }
            if (stream_->sample_duration())
               stream_->duration(info.duration);

            buffer_[0] = stream_->id(info.index);
        }

        void AviTransfer::transfer(
            Sample & sample)
        {
            buffer_[1] = framework::system::BytesOrder::host_to_little_endian(sample.size);
            sample.data.push_front(boost::asio::buffer(buffer_));
            sample.size += 8;
            ctx_->put_sample(*stream_, sample);
        }

        void AviTransfer::on_event(
            MuxEvent const & event)
        {
            if (event.type == event.end) {
                stream_->put_eos();
            }
        }

    } // namespace mux
} // namespace ppbox
