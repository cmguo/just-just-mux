// FlvMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvMuxer.h"
#include "ppbox/mux/transfer/PackageJoinTransfer.h"
#include "ppbox/mux/transfer/StreamSplitTransfer.h"
#include "ppbox/mux/transfer/PtsComputeTransfer.h"
#include "ppbox/mux/flv/FlvAudioTransfer.h"
#include "ppbox/mux/flv/FlvVideoTransfer.h"

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
        }

        FlvMuxer::~FlvMuxer()
        {
        }

        void FlvMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            Transfer * transfer = NULL;
            if (info.type == MEDIA_TYPE_VIDE) {
                if (info.format_type == StreamInfo::video_avc_packet) {
                    // empty
                } else if (info.format_type == StreamInfo::video_avc_byte_stream) {
                    transfer = new StreamSplitTransfer();
                    transfers.push_back(transfer);
                    transfer = new PtsComputeTransfer();
                    transfers.push_back(transfer);
                    transfer = new PackageJoinTransfer();
                    transfers.push_back(transfer);
                }
                transfer = new FlvVideoTransfer(9);
                transfers.push_back(transfer);
            } else if (info.type == MEDIA_TYPE_AUDI) {
                transfer = new FlvAudioTransfer(8);
                transfers.push_back(transfer);
            }
        }

        void FlvMuxer::file_header(
            Sample & sample)
        {
            sample.data.clear();
            sample.time = 0;
            sample.ustime = 0;
            sample.dts = 0;
            sample.cts_delta = 0;
            util::archive::ArchiveBuffer<char> file_header_buf(flv_file_header_buffer_, 13);
            ppbox::avformat::FLVOArchive flv_file_archive(file_header_buf);
            flv_file_archive << flv_header_;
            sample.data.push_back(boost::asio::buffer(
                (boost::uint8_t *)&flv_file_header_buffer_, 13));
            sample.size = 13;
        }

        void FlvMuxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
            boost::uint32_t spec_data_size = 0;

            sample.data.clear();
            sample.time = 0;
            sample.ustime = 0;
            sample.dts = 0;
            sample.cts_delta = 0;

            StreamInfo const & stream_info = streams_[index];
            if (stream_info.type == MEDIA_TYPE_VIDE) {
                spec_data_size = stream_info.format_data.size();
                flv_tag_header_.Type = 0x09;
                flv_tag_header_.Filter = 0;
                flv_tag_header_.Reserved = 0;
                flv_tag_header_.DataSize = framework::system::UInt24(spec_data_size+5);
                flv_tag_header_.Timestamp = framework::system::UInt24(0);
                flv_tag_header_.TimestampExtended = 0;
                flv_tag_header_.StreamID = framework::system::UInt24(0);
                util::archive::ArchiveBuffer<char> buf(video_header_buffer_, 16);
                ppbox::avformat::FLVOArchive flv_archive(buf);
                flv_archive << flv_tag_header_;

                if (stream_info.sub_type == VIDEO_TYPE_AVC1) {
                    flv_video_tag_header_.FrameType = 1;
                    flv_video_tag_header_.CodecID = FlvVideoCodec::H264;
                    flv_video_tag_header_.AVCPacketType = 0;
                } else {
                    flv_video_tag_header_.FrameType = 1;
                    flv_video_tag_header_.CodecID = FlvVideoCodec::H264;
                    flv_video_tag_header_.AVCPacketType = 0;
                }
                flv_video_tag_header_.CompositionTime = framework::system::UInt24(0);
                flv_archive << flv_video_tag_header_;
                sample.data.push_back(boost::asio::buffer(
                    (boost::uint8_t *)&video_header_buffer_, 16));
                sample.data.push_back(boost::asio::buffer(stream_info.format_data));
                video_header_size_ = spec_data_size + 16;
                sample.size = video_header_size_ + 4;
                video_header_size_ = framework::system::BytesOrder::big_endian_to_host_long(video_header_size_);
                sample.data.push_back(boost::asio::buffer(
                    (boost::uint8_t *)&video_header_size_, 4));

            } else if (stream_info.type == MEDIA_TYPE_AUDI) {
                spec_data_size = stream_info.format_data.size();
                flv_tag_header_.Type = 0x08;
                flv_tag_header_.Filter = 0;
                flv_tag_header_.Reserved = 0;
                flv_tag_header_.DataSize = framework::system::UInt24(spec_data_size+2);
                flv_tag_header_.Timestamp = framework::system::UInt24(0);
                flv_tag_header_.TimestampExtended = 0;
                flv_tag_header_.StreamID = framework::system::UInt24(0);
                util::archive::ArchiveBuffer<char> buf(audio_header_buffer_, 13);
                ppbox::avformat::FLVOArchive flv_archive(buf);
                flv_archive << flv_tag_header_;

                switch(stream_info.sub_type)
                {
                case AUDIO_TYPE_MP4A:
                    flv_audio_tag_header_.SoundFormat = 10;
                    break;
                case AUDIO_TYPE_MP1A:
                    flv_audio_tag_header_.SoundFormat = 2;
                    break;
                case AUDIO_TYPE_WMA2:
                    flv_audio_tag_header_.SoundFormat = 11;
                    break;
                default:
                    flv_audio_tag_header_.SoundFormat = 10;
                    break;
                }

                if (stream_info.audio_format.sample_rate >= 44100 ) {
                    flv_audio_tag_header_.SoundRate = 3;
                } else if (stream_info.audio_format.sample_rate >= 22000 ){
                    flv_audio_tag_header_.SoundRate = 2;
                } else if (stream_info.audio_format.sample_rate >= 11000) {
                    flv_audio_tag_header_.SoundRate = 1;
                } else if (stream_info.audio_format.sample_rate >= 5500) {
                    flv_audio_tag_header_.SoundRate = 0;
                } else {
                    flv_audio_tag_header_.SoundRate = 0;
                }

                if (stream_info.audio_format.channel_count <= 1) {
                    // for aac always 1;
                    flv_audio_tag_header_.SoundType = 1;
                } else {
                    flv_audio_tag_header_.SoundType = 1;
                }

                if (8 == stream_info.audio_format.sample_size) {
                    flv_audio_tag_header_.SoundSize = 0;
                } else {
                    flv_audio_tag_header_.SoundSize = 1;
                }
                flv_audio_tag_header_.AACPacketType = 0;

                flv_archive << flv_audio_tag_header_;
                sample.data.push_back(boost::asio::buffer(
                    (boost::uint8_t *)&audio_header_buffer_, 13));
                sample.data.push_back(boost::asio::buffer(stream_info.format_data));
                audio_header_size_ = spec_data_size + 13;
                sample.size = audio_header_size_ + 4;
                audio_header_size_ = framework::system::BytesOrder::big_endian_to_host_long(audio_header_size_);
                sample.data.push_back(boost::asio::buffer(
                    (boost::uint8_t *)&audio_header_size_, 4));
            }
        }

    } // namespace mux
} // namespace ppbox
