// SegmentFilter.h

#include "ppbox/mux/Common.h"
#include "ppbox/mux/filter/SegmentFilter.h"

using namespace ppbox::avformat;

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        SegmentFilter::SegmentFilter()
            : video_track_(boost::uint32_t(-1))
            , segent_end_time_(0)
            , fisrt_idr_timestamp_us_(boost::uint64_t(-1))
            , is_save_sample_(false)
        {
        }

        SegmentFilter::~SegmentFilter()
        {
        }

        error_code SegmentFilter::open(
            MediaInfo const & media_info, 
            std::vector<StreamInfo> const & streams, 
            boost::system::error_code & ec)
        {
            if (Filter::open(media_info, streams, ec))
                return ec;
            video_track_ = boost::uint32_t(-1);
            for (size_t i = 0; i < streams.size(); ++i) {
                if (streams[i].type == MEDIA_TYPE_VIDE) {
                    video_track_ = i;
                    break;
                }
            }
            return ec;
        }

        error_code SegmentFilter::get_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (is_save_sample_) {
                sample = sample_;
                is_save_sample_ = false;
                ec.clear();
            } else {
                if (Filter::get_sample(sample, ec))
                    return ec;
            }
            if (fisrt_idr_timestamp_us_ == boost::uint64_t(-1)
                && (video_track_ == boost::uint32_t(-1)
                    || (sample.itrack == video_track_
                    && (sample.flags & Sample::sync)))) {
                        // fisrt_idr_timestamp_us_ = sample.ustime;
                        fisrt_idr_timestamp_us_ = 0;
                        segent_end_time_ += fisrt_idr_timestamp_us_;
            }
            //std::cout << "[SegmentFilter::get_sample] sample.ustime = " << sample.ustime << " segent_end_time_ = " << segent_end_time_ << std::endl;
            if (sample.ustime >= segent_end_time_
                && (video_track_ == boost::uint32_t(-1)
                    || (sample.itrack == video_track_
                    && (sample.flags & Sample::sync)))) {
                        ec = error::mux_segment_end;
                        is_save_sample_ = true;
                        sample_ = sample;
            }
            return ec;
        }

        void SegmentFilter::set_end_time(
            boost::uint64_t time)
        {
            if (fisrt_idr_timestamp_us_ == boost::uint64_t(-1))
                segent_end_time_ = time;
            else
                segent_end_time_ = time + fisrt_idr_timestamp_us_;
        }

        void SegmentFilter::reset()
        {
            fisrt_idr_timestamp_us_ = boost::uint64_t(-1);
            is_save_sample_ = false;
            segent_end_time_ = 0;
        }

    } // namespace mux
} // namespace ppbox
