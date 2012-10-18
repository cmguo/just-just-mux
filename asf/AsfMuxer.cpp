// AsfMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/asf/AsfMuxer.h"
#include "ppbox/mux/asf/AsfTransfer.h"
#include "ppbox/mux/transfer/MergeTransfer.h"
#include "ppbox/mux/transfer/PackageSplitTransfer.h"
#include "ppbox/mux/transfer/StreamJoinTransfer.h"

#include <ppbox/avformat/asf/AsfGuid.h>
#include <ppbox/avformat/asf/AsfObjectType.h>
#include <ppbox/avformat/codec/AvcConfig.h>
using namespace ppbox::avformat;

#include <util/archive/BigEndianBinaryOArchive.h>

namespace ppbox
{
    namespace mux
    {

        AsfMuxer::AsfMuxer()
            : stream_number_(0)
             , transfer_(NULL)
        {
        }

        AsfMuxer::~AsfMuxer()
        {
        }

        void AsfMuxer::add_stream(
            StreamInfo & infoex)
        {
             Transfer * transfer = NULL;
             if (infoex.type == MEDIA_TYPE_VIDE) {
                 if (infoex.format_type == StreamInfo::video_avc_packet) {
                     transfer = new PackageSplitTransfer();
                     infoex.transfers.push_back(transfer);
                     transfer = new StreamJoinTransfer();
                     infoex.transfers.push_back(transfer);
                 }
             }
             if (transfer_ == NULL)
                 transfer_ = new AsfTransfer(*this);
             infoex.transfers.push_back(new MergeTransfer(transfer_));

            //structure ASF_Stream_Properties_Object
            stream_number_++;
            ASF_Stream_Properties_Object streams_object;
            streams_object.StreamType = 
                infoex.type == MEDIA_TYPE_VIDE ? ASF_Video_Media : ASF_Audio_Media;

//             streams_object.ErrorCorrectionType =
//                 info.type == MEDIA_TYPE_VIDE ? ASF_No_Error_Correction : ASF_Audio_Spread;
            streams_object.ErrorCorrectionType = ASF_No_Error_Correction;
            streams_object.Flag.StreamNumber = stream_number_;

            if (ASF_Video_Media == streams_object.StreamType) {
                streams_object.Video_Media_Type.EncodeImageWidth  = infoex.video_format.width;
                streams_object.Video_Media_Type.EncodeImageHeight = infoex.video_format.height;

                streams_object.Video_Media_Type.FormatData.ImageWidth  = infoex.video_format.width;
                streams_object.Video_Media_Type.FormatData.ImageHeight = infoex.video_format.height;
                streams_object.Video_Media_Type.FormatData.BitsPerPixelCount = 24;
                if (infoex.sub_type == VIDEO_TYPE_AVC1) {
                    streams_object.Video_Media_Type.FormatData.CompressionID = 
                        MAKE_FOURC_TYPE('H', '2', '6', '4');
                }
                else
                    streams_object.Video_Media_Type.FormatData.CompressionID = 0;
                AvcConfig avc_config(&infoex.format_data.at(0), infoex.format_data.size());
                avc_config.creat();
                std::vector<boost::uint8_t> vec_0001;
                vec_0001.push_back(0);
                vec_0001.push_back(0);
                vec_0001.push_back(0);
                vec_0001.push_back(1);
                std::vector<boost::uint8_t> sps_pps;
                sps_pps = vec_0001;
                sps_pps.insert(sps_pps.end(), avc_config.sequence_parameters()[0].begin(), avc_config.sequence_parameters()[0].end());
                sps_pps.insert(sps_pps.end(), vec_0001.begin(), vec_0001.end());
                sps_pps.insert(sps_pps.end(), avc_config.picture_parameters()[0].begin(), avc_config.picture_parameters()[0].end());
                streams_object.Video_Media_Type.FormatData.CodecSpecificData = sps_pps;
                streams_object.Video_Media_Type.FormatData.FormatDataSize = 
                    streams_object.Video_Media_Type.FormatData.CodecSpecificData.size() + 40;
                streams_object.Video_Media_Type.FormatDataSize = 
                    (boost::uint16_t)streams_object.Video_Media_Type.FormatData.FormatDataSize;
                streams_object.ObjLength = 89 + streams_object.Video_Media_Type.FormatDataSize;
                streams_object.TypeSpecificDataLength =
                    11 + streams_object.Video_Media_Type.FormatDataSize;

                ASFOArchiveChar  oarchv(stream_buf_);
                oarchv << streams_object;
            }
            else
            {
                if (infoex.sub_type == AUDIO_TYPE_MP4A)
                {
                    streams_object.Audio_Media_Type.CodecId = 255;
                }
                else if (infoex.sub_type == AUDIO_TYPE_WMA2)
                {
                    streams_object.Audio_Media_Type.CodecId = 353;
                }
                else
                {
                    streams_object.Audio_Media_Type.CodecId = 0;
                }
                streams_object.Audio_Media_Type.NumberOfChannels = (boost::uint16_t)infoex.audio_format.channel_count;
                streams_object.Audio_Media_Type.SamplesPerSecond = infoex.audio_format.sample_rate;
                //streams_object.Audio_Media_Type.AverageNumberOfBytesPerSecond = 4000;
                //streams_object.Audio_Media_Type.BlockAlignment = 
                //    info.audio_format.channel_count * info.audio_format.sample_size / 8;
                //streams_object.Audio_Media_Type.BitsPerSample = info.audio_format.sample_size;
                streams_object.Audio_Media_Type.CodecSpecificDataSize = 
                    infoex.format_data.size();
                streams_object.Audio_Media_Type.CodecSpecificData = infoex.format_data;

                streams_object.ObjLength = 96 + streams_object.Audio_Media_Type.CodecSpecificDataSize;

                streams_object.TypeSpecificDataLength = 
                    18 + streams_object.Audio_Media_Type.CodecSpecificDataSize;
                ASFOArchiveChar  oarchv(stream_buf_);
                oarchv << streams_object;
            }
        }

        void AsfMuxer::stream_header(
            boost::uint32_t index, 
            Sample & tag)
        {
            tag.data.clear();
        }

        void AsfMuxer::file_header(
            Sample & tag)
        {
            if(0 != file_buf_.size()) {
                head_buf_.reset();
                extension_buf_.reset();
                file_buf_.reset();
                data_buf_.reset();
            }
            tag.data.clear();

            //ASF_Header_Extension_Object
            ASF_Header_Extension_Object extension_object;
            extension_object.ObjectSize = 46;
            //extension_object.HeaderExtensionDataSize = 0;
            ASFOArchiveChar oar_ext(extension_buf_);
            oar_ext << extension_object;

            //ASF_File_Properties_Object
            ASF_File_Properties_Object file_object;
            file_object.ObjLength = 104;
            file_object.FileId.generate();
            file_object.FileSize = 104;
            file_object.PlayDuration = media_info_.duration;
            file_object.Flag.BroadcastFlag = 1;
            //file_object.Flag.SeekableFlag = 0;
            file_object.Flag.Reserved = 1;
            file_object.MinimumDataPacketSize = transfer_->packet_length();
            file_object.MaximumDataPacketSize = transfer_->packet_length();

            ASFOArchiveChar oar_file(file_buf_);
            oar_file << file_object;

            //Header_Object
            ASF_Header_Object header_object;
            header_object.ObjLength = file_buf_.size() + stream_buf_.size() + extension_buf_.size() +30;
            header_object.NumOfHeaderObject = stream_number_ + 1;
            ASFOArchiveChar oar_head(head_buf_);
            oar_head << header_object;

            //Data_Object_header
            ASF_Data_Object data_object;
            data_object.ObjLength =0x32;
            data_object.FileId.generate();
            //data_object.TotalDataPackets = 1;
            ASFOArchiveChar oar_data(data_buf_);
            oar_data << data_object;

            tag.data.push_back( head_buf_.data() );
            tag.data.push_back( file_buf_.data() );
            tag.data.push_back( extension_buf_.data() );
            tag.data.push_back( stream_buf_.data() );
            tag.data.push_back( data_buf_.data() );
        }

        boost::system::error_code AsfMuxer::time_seek(
            boost::uint64_t & time,
            boost::system::error_code & ec)
        {
            MuxerBase::time_seek(time, ec);
            if (!ec || ec == boost::asio::error::would_block) {
                transfer_->on_seek(time);
            }
            return ec;
        }

    } // namespace mux
} // namespace ppbox
