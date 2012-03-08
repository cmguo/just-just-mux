//MkvMux.cpp
#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/MergeTransfer.h"
#include "ppbox/mux/transfer/PackageSplitTransfer.h"
#include "ppbox/mux/transfer/StreamJoinTransfer.h"

#include <ppbox/mux/mkv/MkvMux.h>
#include <ppbox/mux/mkv/MkvTransfer.h>

using namespace ppbox::demux;
using namespace ppbox::avformat;
namespace ppbox
{
    namespace mux
    {

        MkvMux::MkvMux()
            : stream_number_(0)
             , transfer_(NULL)
        {
        }

        MkvMux::~MkvMux()
        {
        }

        void MkvMux::add_stream(
            MediaInfoEx & mediainfoex)
        {
            stream_number_++;

            Transfer * transfer = NULL;
            //if (mediainfoex.type == MEDIA_TYPE_VIDE) {
                //if (mediainfoex.format_type == MediaInfo::video_avc_packet) {
                    //transfer = new PackageSplitTransfer();
                    //mediainfoex.transfers.push_back(transfer);
                    //transfer = new StreamJoinTransfer();
                    //mediainfoex.transfers.push_back(transfer);
                //}
            //}
            if (transfer_ == NULL)
                transfer_ = new MkvTransfer();
            mediainfoex.transfers.push_back(new MergeTransfer(transfer_));

            //每添加一个流需要添加一个track_entry
            MKV_Track_Entry track_entry;
            track_entry.TrackNumber = mediainfoex.index + 1;
            std::vector<boost::uint8_t> tem(4, (boost::uint8_t)(mediainfoex.index + 1));
            track_entry.TrackUID = tem;
            track_entry.Language = "eng";
            if (mediainfoex.type == MEDIA_TYPE_VIDE) {
                track_entry.TrackType = VIDEO_TRACKE_TYPE_VALUE;
                track_entry.CodecID = "V_MPEG4/ISO/AVC";
                track_entry.CodecPrivate = mediainfoex.format_data;
                track_entry.Video.PixelWidth = mediainfoex.video_format.width;
                track_entry.Video.PixelHeight = mediainfoex.video_format.height;
                track_entry.Video.size = track_entry.Video.data_size();
            } else if (mediainfoex.type == MEDIA_TYPE_AUDI){
                track_entry.TrackType = AUDIO_TRACKE_TYPE_VALUE;
                track_entry.CodecID = "A_AAC";
                track_entry.CodecPrivate = mediainfoex.format_data;
                track_entry.Audio.SamplingFrequency = 
                    (float)mediainfoex.audio_format.sample_rate;
                track_entry.Audio.Channels = 
                    mediainfoex.audio_format.channel_count;
                //track_entry.Audio.BitDepth = 
                //    mediainfoex.audio_format.sample_size * 8;
                track_entry.Audio.size = track_entry.Audio.data_size();
            } else {
                track_entry.TrackType = SUBTITLE_TRACKE_TYPE_VALUE;
                track_entry.CodecID = "S_TEXT/UTF-8";
            }

            track_entry.size = track_entry.data_size();

            MKVOArchive oar(track_buf_);
            oar << track_entry;

        }

        void MkvMux::stream_header(
            boost::uint32_t index, 
            ppbox::demux::Sample & tag)
        {
            tag.data.clear();
        }

        void MkvMux::file_header(
            ppbox::demux::Sample & tag)
        {
            tag.data.clear();
            if(ebml_buf_.size() != 0) {
                ebml_buf_.reset();
                segment_buf_.reset();
                track_head_buf_.reset();
            }

            //EBML_buf_
            EBML_Object ebml;
//             ebml.EBMLVersion = 1;
//             ebml.EBMLReadVersion = 1;
//             ebml.EBMLMaxIDLength = 4;
//             ebml.EBMLMaxSizeLength = 8;
//             ebml.DocType = "matroska";
            ebml.DocTypeVersion = 2;
            ebml.DocTypeReadVersion = 2; 

            ebml.size = ebml.data_size();

            MKVOArchive oar_ebml(ebml_buf_);
            oar_ebml << ebml;

            //Segment_buf_
            MKV_Segment_Header segm_head;
            segm_head.size = framework::system::VariableNumber<boost::uint32_t>::unknown(1);
            MKV_Segment_Info segm_info;
            segm_info.Time_Code_Scale = 1000000;
            segm_info.Duration = (float)(media_info_.duration);
            segm_info.Muxing_App = "ppbox libmatroska v1.0.0";
            segm_info.Writing_App = "mkvmerge 1.0.0";
            segm_info.size = segm_info.data_size();

            MKVOArchive oar_segment(segment_buf_);
            oar_segment << segm_head << segm_info;

            //track_head
            MKV_Tracks_Header tracks_head;
            tracks_head.size = track_buf_.size();

            MKVOArchive oar_track_head(track_head_buf_);
            oar_track_head << tracks_head;

            tag.data.push_back( ebml_buf_.data() );
            tag.data.push_back( segment_buf_.data() );
            tag.data.push_back( track_head_buf_.data() );
            tag.data.push_back( track_buf_.data() );
        }

        boost::system::error_code MkvMux::seek(
            boost::uint32_t & time,
            boost::system::error_code & ec)
        {
            Muxer::seek(time, ec);
            if (!ec || ec == boost::asio::error::would_block) {
                transfer_->on_seek();
            }
            return ec;
        }
    }//mux
}//ppbox

