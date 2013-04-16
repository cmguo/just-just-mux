// FlvMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvMuxer.h"
#include "ppbox/mux/flv/FlvAudioTransfer.h"
#include "ppbox/mux/flv/FlvVideoTransfer.h"
#include "ppbox/mux/transfer/H264PackageJoinTransfer.h"
#include "ppbox/mux/transfer/H264StreamSplitTransfer.h"
#include "ppbox/mux/transfer/H264PtsComputeTransfer.h"
#include "ppbox/mux/transfer/MpegAudioAdtsDecodeTransfer.h"

#include <ppbox/avformat/Format.h>
using namespace ppbox::avformat;

#include <framework/system/BytesOrder.h>
using namespace framework::system;

using namespace boost::system;

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
            std::vector<Transfer *> & transfers)
        {
            Transfer * transfer = NULL;
            if (info.type == MEDIA_TYPE_VIDE) {
                if (info.sub_type == VIDEO_TYPE_AVC1) {
                    if (info.format_type == FormatType::video_avc_packet) {
                        // empty
                    } else if (info.format_type == FormatType::video_avc_byte_stream) {
                        transfer = new H264StreamSplitTransfer();
                        transfers.push_back(transfer);
                        transfer = new H264PtsComputeTransfer();
                        transfers.push_back(transfer);
                        transfer = new H264PackageJoinTransfer();
                        transfers.push_back(transfer);
                    }
                }
                FlvVideoTransfer * transfer = new FlvVideoTransfer;
                transfers_.push_back(transfer);
                transfers.push_back(transfer);
                flv_header_.TypeFlagsVideo = 1;
                meta_data_.width = info.video_format.width;
                meta_data_.height= info.video_format.height;
                meta_data_.framerate = info.video_format.frame_rate;
            } else if (info.type == MEDIA_TYPE_AUDI) {
                if (info.sub_type == AUDIO_TYPE_MP4A) {
                    if (info.format_type == FormatType::audio_aac_adts) {
                        transfer = new MpegAudioAdtsDecodeTransfer();
                        transfers.push_back(transfer);
                    }
                }
                FlvAudioTransfer * transfer = new FlvAudioTransfer;
                transfers_.push_back(transfer);
                transfers.push_back(transfer);
                flv_header_.TypeFlagsAudio = 1;
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
                //meta_data_.duration = media_info_.duration;
            }

            {
                FormatBuffer buf(meta_data_buffer_, sizeof(meta_data_buffer_));
                ppbox::avformat::FlvOArchive archive(buf);
                FlvDataTag tag;
                tag.Name = "onMetaData";
                meta_data_.to_data(tag.Value);
                archive << tag;
                sample.data.push_back(buf.data());
                sample.size = buf.size();
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
