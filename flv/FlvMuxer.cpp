// FlvMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvMuxer.h"
#include "ppbox/mux/flv/FlvAudioTransfer.h"
#include "ppbox/mux/flv/FlvVideoTransfer.h"

#include <ppbox/avformat/flv/FlvEnum.h>
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        FlvMuxer::FlvMuxer()
        {
            meta_data_transfer_ = new FlvTransfer(FlvTagType::DATA);
        }

        FlvMuxer::~FlvMuxer()
        {
            delete meta_data_transfer_;
        }

        void FlvMuxer::add_stream(
            StreamInfo & info, 
            FilterPipe & pipe)
        {
            if (info.type == StreamType::VIDE) {
                FlvVideoTransfer * transfer = new FlvVideoTransfer;
                transfers_.push_back(transfer);
                pipe.insert(transfer);
                flv_header_.TypeFlagsVideo = 1;
                meta_data_.hasvideo = true;
                meta_data_.width = info.video_format.width;
                meta_data_.height= info.video_format.height;
                meta_data_.framerate = info.video_format.frame_rate();
            } else if (info.type == StreamType::AUDI) {
                FlvAudioTransfer * transfer = new FlvAudioTransfer;
                transfers_.push_back(transfer);
                pipe.insert(transfer);
                flv_header_.TypeFlagsAudio = 1;
                meta_data_.hasaudio = true;
                meta_data_.audiosamplerate = info.audio_format.sample_rate;
                meta_data_.audiosamplesize = info.audio_format.sample_size;
                //meta_data_.stereo = info.audio_format.channel_count;
            }
        }

        void FlvMuxer::file_header(
            Sample & sample)
        {
            FormatBuffer buf(header_buffer_, sizeof(header_buffer_));
            ppbox::avformat::FlvOArchive archive(buf);
            archive << flv_header_;

            meta_data_.author = "ppbox muxer";
            meta_data_.copyright = "ppbox 2013";
            meta_data_.description = media_info_.name;

            if (media_info_.duration != ppbox::data::invalid_size) {
                meta_data_.duration = media_info_.duration;
            }

            get_seek_points(meta_data_.keyframes);

            {
                //FormatBuffer buf(meta_data_buffer_, sizeof(meta_data_buffer_));
                ppbox::avformat::FlvOArchive archive(meta_data_buffer_);
                FlvDataTag tag;
                tag.Name = "onMetaData";
                meta_data_.to_data(tag.Value);
                archive << tag;
                sample.data.push_back(meta_data_buffer_.data());
                sample.size = meta_data_buffer_.size();
                meta_data_transfer_->transfer(sample);
            }

            sample.data.push_front(buf.data());
            sample.size += buf.size();
        }

        void FlvMuxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
            StreamInfo const & stream_info = streams_[index];
            transfers_[index]->stream_header(stream_info, sample);
        }

    } // namespace mux
} // namespace ppbox
