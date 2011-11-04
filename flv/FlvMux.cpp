// FlvMux.cpp
#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvMux.h"
#include "ppbox/mux/transfer/PackageJoinTransfer.h"
#include "ppbox/mux/transfer/StreamSplitTransfer.h"
#include "ppbox/mux/flv/FlvAudioTransfer.h"
#include "ppbox/mux/flv/FlvVideoTransfer.h"

#include <ppbox/demux/Demuxer.h>
using namespace ppbox::demux;

#include <framework/system/BytesOrder.h>
using namespace framework::system;
using namespace boost::system;

namespace ppbox
{
    namespace mux
    {
        FlvMux::FlvMux()
        {
            memset(&flvheader_, 0, sizeof(FlvHeader));
            flvheader_.Signature[0]  = 'F';
            flvheader_.Signature[1]  = 'L';
            flvheader_.Signature[2]  = 'V';
            flvheader_.Version       = 0x01;
            flvheader_.Flags         = 0x05;
            flvheader_.DataOffset[0] = 0x0;
            flvheader_.DataOffset[1] = 0x0;
            flvheader_.DataOffset[2] = 0x0;
            flvheader_.DataOffset[3] = 0x9;
            flvheader_.Tag0Size[0] = 0x0;
            flvheader_.Tag0Size[1] = 0x0;
            flvheader_.Tag0Size[2] = 0x0;
            flvheader_.Tag0Size[3] = 0x0;
        }

        FlvMux::~FlvMux()
        {
        }

        void FlvMux::add_stream(
            MediaInfo & mediainfo,
            std::vector<Transfer *> & transfers)
        {
            Transfer * transfer = NULL;
            if (mediainfo.type == ppbox::demux::MEDIA_TYPE_VIDE) {
                if (mediainfo.format_type == MediaInfo::video_avc_packet) {
                    transfer = new FlvVideoTransfer();
                    transfers.push_back(transfer);
                } else if (mediainfo.format_type == MediaInfo::video_avc_byte_stream) {
                    transfer = new StreamSplitTransfer();
                    transfers.push_back(transfer);
                    transfer = new PackageJoinTransfer();
                    transfers.push_back(transfer);
                    transfer = new FlvVideoTransfer();
                    transfers.push_back(transfer);
                }
            } else {
                transfer = new FlvAudioTransfer();
                transfers.push_back(transfer);
            }
        }

        void FlvMux::head_buffer(
            ppbox::demux::Sample & tag)
        {
            boost::uint32_t offset = 0;
            boost::uint32_t head_size = sizeof(FlvHeader);
            file_head_buffer_.resize(head_size);
            memcpy(&file_head_buffer_.at(0), (boost::uint8_t *)&flvheader_, sizeof(FlvHeader));
            offset += sizeof(FlvHeader);
            boost::uint32_t spec_data_size = 0;

            if (mediainfo().video_index != boost::uint32_t(-1)) {
                ppbox::demux::MediaInfo const & stream_info = 
                    mediainfo().stream_infos[mediainfo().video_index];
                spec_data_size = stream_info.format_data.size();
                boost::uint32_t head_size = sizeof(VideoTagHeader)+11;
                file_head_buffer_.resize(offset+head_size+spec_data_size+4);
                flvtag_.TagType = TAG_TYPE_VIDEO;
                setTagSizeAndTimestamp(flvtag_, spec_data_size+sizeof(VideoTagHeader), 0);
                memcpy(&file_head_buffer_.at(0)+offset, (boost::uint8_t *)&flvtag_, 11);
                offset += 11;
                VideoTagHeader videotagheader;
                if (stream_info.sub_type == ppbox::demux::VIDEO_TYPE_AVC1) {
                    videotagheader.VideoAttribute = 0x17;
                    videotagheader.AVCPacketType = 0x0;
                } else {
                    videotagheader.VideoAttribute = 0x17;
                    videotagheader.AVCPacketType = 0x0;
                }
                memset(&videotagheader.CompositionTime, 0, sizeof(videotagheader.CompositionTime));
                memcpy( &file_head_buffer_.at(0)+offset,
                    (boost::uint8_t *)&videotagheader,
                    5);
                offset += 5;
                memcpy(&file_head_buffer_.at(0)+offset,
                    &stream_info.format_data.at(0),
                    spec_data_size);
                offset += spec_data_size;
                boost::uint32_t PreviousTagSize = spec_data_size + sizeof(VideoTagHeader) + 11;
                PreviousTagSize = BytesOrder::host_to_big_endian_long(PreviousTagSize);
                memcpy(&file_head_buffer_.at(0)+offset,
                    (unsigned char const *)&PreviousTagSize,
                    4);
                offset += 4;
            }

            if (mediainfo().audio_index != boost::uint32_t(-1)) {
                ppbox::demux::MediaInfo const & stream_info = 
                    mediainfo().stream_infos[mediainfo().audio_index];
                boost::uint32_t head_size = sizeof(AudioTagHeader)+11;
                boost::uint32_t spec_data_size =  stream_info.format_data.size();
                file_head_buffer_.resize(offset+head_size+spec_data_size+4);
                flvtag_.TagType = TAG_TYPE_AUDIO;
                setTagSizeAndTimestamp(flvtag_, spec_data_size+sizeof(AudioTagHeader), 0);
                memcpy(&file_head_buffer_.at(0)+offset, (boost::uint8_t *)&flvtag_, 11);
                offset += 11;
                AudioTagHeader audiotagheader;
                if (stream_info.sub_type == ppbox::demux::AUDIO_TYPE_MP4A) {
                    audiotagheader.SoundAttribute = 0xAF;
                    audiotagheader.AACPacketType = 0x0;
                } else if (stream_info.sub_type == ppbox::demux::AUDIO_TYPE_WMA2) {
                    audiotagheader.SoundAttribute = 0xDF;
                    audiotagheader.AACPacketType = 0x0;
                } else {
                    audiotagheader.SoundAttribute = 0xAF;
                    audiotagheader.AACPacketType = 0x0;
                }
                memcpy(&file_head_buffer_.at(0)+offset,
                    (boost::uint8_t *)&audiotagheader,
                    2);
                offset += 2;
                memcpy(&file_head_buffer_.at(0)+offset,
                    &stream_info.format_data.at(0),
                    spec_data_size);
                offset += spec_data_size;
                boost::uint32_t PreviousTagSize = spec_data_size + sizeof(AudioTagHeader) + 11;
                PreviousTagSize = BytesOrder::host_to_big_endian_long(PreviousTagSize);
                memcpy(&file_head_buffer_.at(0)+offset,
                    (unsigned char const *)&PreviousTagSize,
                    4);
                offset += 4;
            }
            tag.data.clear();
            tag.data.push_back(boost::asio::buffer(&file_head_buffer_.at(0), file_head_buffer_.size()));
        }

    } // namespace mux
} // namespace ppbox
