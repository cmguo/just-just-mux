// MkvTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/mkv/MkvTransfer.h"

#include <ppbox/avformat/mkv/MkvObjectType.h>
#include <ppbox/avformat/mkv/MkvArchive.h>
#include <ppbox/avformat/mkv/MkvFormat.h>
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        static boost::uint32_t const TIME_SCALE = 1000;

        MkvTransfer::MkvTransfer()
            : TimeScaleTransfer(TIME_SCALE)
            , add_cluster_flag_(0)
            , time_code_(0)
        {
        }

        MkvTransfer::~MkvTransfer()
        {
        }

        void MkvTransfer::transfer(
            StreamInfo & info)
        {
            TimeScaleTransfer::transfer(info);
        }

        void MkvTransfer::transfer(
            Sample & sample)
        {
            TimeScaleTransfer::transfer(sample);

            FormatBuffer buf(block_head_buf_, sizeof(block_head_buf_));
            MkvOArchive oa(buf);

            if (0 == add_cluster_flag_) {
                MkvCluster cluster;
                cluster.Size = framework::system::VariableNumber<boost::uint32_t>::unknown(1);
                cluster.TimeCode = time_code_ = sample.dts;
                oa << cluster;
                add_cluster_flag_ = 100;
            }

            MkvSimpleBlock simple_block;
            simple_block.TrackNumber = sample.itrack + 1;
            simple_block.TimeCode = (boost::uint16_t)(sample.dts - time_code_);
            if (Sample::f_sync & sample.flags)
                simple_block.Keyframe = 1;
            else
                simple_block.Keyframe = 0;
            simple_block.Size = simple_block.TrackNumber.size() + 3 + sample.size;
            oa << simple_block;

            sample.size += buf.size();
            sample.data.push_front(buf.data());
            --add_cluster_flag_;
        }

        void MkvTransfer::on_seek(
            boost::uint64_t time)
        {
            TimeScaleTransfer::on_seek(time);

            add_cluster_flag_ = 0;
            time_code_ = 0;
        }

        void MkvTransfer::file_header(
            MediaInfo const & info, 
            size_t stream_obj_size, 
            boost::asio::streambuf & buf)
        {
            //EBML_buf_
            MkvFile file;

            //Segment_buf_
            EBML_ElementHeader segm_head(MkvSegment::StaticId);
            segm_head.Size = vint::unknown(1);

            MkvSegmentInfo segm_info;
            segm_info.Time_Code_Scale = 1000000000 / TIME_SCALE;
            segm_info.Duration = (float)(info.duration);
            segm_info.Muxing_App = "ppbox v1.0.0";
            segm_info.Writing_App = "mkvmerge 1.0.0";

            //track_head
            EBML_ElementHeader tracks_head(MkvTracks::StaticId, stream_obj_size);

            EBML_DataSizeArchive dsa;
            dsa >> file >> segm_info;

            util::archive::BigEndianBinaryOArchive<> oa(buf);
            oa << file;
            oa << segm_head;
            oa << segm_info;
            oa << tracks_head;
        }

        void MkvTransfer::stream_header(
            StreamInfo const & info, 
            boost::asio::streambuf & buf)
        {
            MkvFormat mkv;
            CodecInfo const * codec = mkv.codec_from_codec(info.type, info.sub_type);
            if (codec == NULL) {
                return;
            }
            //每添加一个流需要添加一个track_entry
            MkvTrackEntry track_entry;
            track_entry.TrackNumber = info.index + 1;
            std::vector<boost::uint8_t> tem(4, (boost::uint8_t)(info.index + 1));
            track_entry.TrackUID = tem;
            track_entry.Language = "eng";
            if (info.type == StreamType::VIDE) {
                track_entry.TrackType = MkvTrackType::VIDEO;
                track_entry.CodecID = (char const *)codec->format;
                track_entry.CodecPrivate = info.format_data;
                track_entry.Video.PixelWidth = info.video_format.width;
                track_entry.Video.PixelHeight = info.video_format.height;
                track_entry.Video.Size = track_entry.Video.data_size();
            } else if (info.type == StreamType::AUDI){
                track_entry.TrackType = MkvTrackType::AUDIO;
                track_entry.CodecID = (char const *)codec->format;
                track_entry.CodecPrivate = info.format_data;
                track_entry.Audio.SamplingFrequency = 
                    (float)info.audio_format.sample_rate;
                track_entry.Audio.Channels = 
                    info.audio_format.channel_count;
                //track_entry.Audio.BitDepth = 
                //    mediainfoex.audio_format.sample_size * 8;
                track_entry.Audio.Size = track_entry.Audio.data_size();
            } else {
                track_entry.TrackType = MkvTrackType::SUBTITLE;
                track_entry.CodecID = "S_TEXT/UTF-8";
            }

            EBML_DataSizeArchive dsa;
            dsa >> track_entry;

            size_t old_size = buf.size();

            util::archive::BigEndianBinaryOArchive<> oa(buf);
            oa << track_entry;

            assert(old_size + track_entry.byte_size() == buf.size());
        }

    } // namespace mux
} // namespace ppbox

