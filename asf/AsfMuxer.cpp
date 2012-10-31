// AsfMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/asf/AsfMuxer.h"
#include "ppbox/mux/asf/AsfTransfer.h"
#include "ppbox/mux/transfer/MergeTransfer.h"
#include "ppbox/mux/transfer/PackageSplitTransfer.h"
#include "ppbox/mux/transfer/StreamJoinTransfer.h"

#include <ppbox/avformat/asf/AsfGuid.h>
#include <ppbox/avformat/asf/AsfObjectType.h>
#include <ppbox/avformat/codec/avc/AvcConfig.h>
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
            StreamInfo & info)
        {
             Transfer * transfer = NULL;
             if (info.type == MEDIA_TYPE_VIDE) {
                 if (info.format_type == StreamInfo::video_avc_packet) {
                     transfer = new PackageSplitTransfer();
                     add_transfer(info.index, *transfer);
                     transfer = new StreamJoinTransfer();
                     add_transfer(info.index, *transfer);
                 }
             }
             if (transfer_ == NULL)
                 transfer_ = new AsfTransfer(*this);
             transfer=  new MergeTransfer(transfer_);
             add_transfer(info.index, *transfer);

            //structure ASF_Stream_Properties_Object
            stream_number_++;
            ASF_Stream_Properties_Object streams_object;
            streams_object.StreamType = 
                info.type == MEDIA_TYPE_VIDE ? ASF_Video_Media : ASF_Audio_Media;

//             streams_object.ErrorCorrectionType =
//                 info.type == MEDIA_TYPE_VIDE ? ASF_No_Error_Correction : ASF_Audio_Spread;
            streams_object.ErrorCorrectionType = ASF_No_Error_Correction;
            streams_object.Flag.StreamNumber = stream_number_;

            if (ASF_Video_Media == streams_object.StreamType) {
                streams_object.Video_Media_Type.EncodeImageWidth  = info.video_format.width;
                streams_object.Video_Media_Type.EncodeImageHeight = info.video_format.height;

                streams_object.Video_Media_Type.FormatData.ImageWidth  = info.video_format.width;
                streams_object.Video_Media_Type.FormatData.ImageHeight = info.video_format.height;
                streams_object.Video_Media_Type.FormatData.BitsPerPixelCount = 24;
                if (info.sub_type == VIDEO_TYPE_AVC1) {
                    streams_object.Video_Media_Type.FormatData.CompressionID = MAKE_FOURC_TYPE('H', '2', '6', '4');
                } else {
                    streams_object.Video_Media_Type.FormatData.CompressionID = 0;
                }
                {
                    AvcConfig avc_config(info.format_data);
                    avc_config.to_es_data(
                        streams_object.Video_Media_Type.FormatData.CodecSpecificData);
                }
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
                if (info.sub_type == AUDIO_TYPE_MP4A)
                {
                    streams_object.Audio_Media_Type.CodecId = 255;
                }
                else if (info.sub_type == AUDIO_TYPE_WMA2)
                {
                    streams_object.Audio_Media_Type.CodecId = 353;
                }
                else
                {
                    streams_object.Audio_Media_Type.CodecId = 0;
                }
                streams_object.Audio_Media_Type.NumberOfChannels = (boost::uint16_t)info.audio_format.channel_count;
                streams_object.Audio_Media_Type.SamplesPerSecond = info.audio_format.sample_rate;
                //streams_object.Audio_Media_Type.AverageNumberOfBytesPerSecond = 4000;
                //streams_object.Audio_Media_Type.BlockAlignment = 
                //    info.audio_format.channel_count * info.audio_format.sample_size / 8;
                //streams_object.Audio_Media_Type.BitsPerSample = info.audio_format.sample_size;
                streams_object.Audio_Media_Type.CodecSpecificDataSize = 
                    info.format_data.size();
                streams_object.Audio_Media_Type.CodecSpecificData = info.format_data;

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

        bool AsfMuxer::time_seek(
            boost::uint64_t & time,
            boost::system::error_code & ec)
        {
            MuxerBase::time_seek(time, ec);
            if (!ec || ec == boost::asio::error::would_block) {
                transfer_->on_seek(time);
            }
            return !ec;
        }

    } // namespace mux
} // namespace ppbox
