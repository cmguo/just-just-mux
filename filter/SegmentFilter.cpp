// SegmentFilter.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/filter/SegmentFilter.h"
#include "ppbox/mux/MuxError.h"

#include <ppbox/demux/base/DemuxError.h>

using namespace ppbox::avformat;

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        SegmentFilter::SegmentFilter()
            : video_track_(boost::uint32_t(-1))
            , segent_end_time_(0)
            , is_save_sample_(false)
            , is_eof_(false)
        {
        }

        SegmentFilter::~SegmentFilter()
        {
        }

        bool SegmentFilter::open(
            MediaInfo const & media_info, 
            std::vector<StreamInfo> const & streams, 
            boost::system::error_code & ec)
        {
            if (!Filter::open(media_info, streams, ec))
                return false;
            video_track_ = boost::uint32_t(-1);
            for (size_t i = 0; i < streams.size(); ++i) {
                if (streams[i].type == MEDIA_TYPE_VIDE) {
                    video_track_ = i;
                    break;
                }
            }
            return true;
        }

        bool SegmentFilter::get_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (is_save_sample_) {
                sample = sample_;
                is_save_sample_ = false;
                ec.clear();
            } else if (is_eof_) {
                ec = ppbox::demux::error::no_more_sample;
                return false;
            } else {
                if (!Filter::get_sample(sample, ec)) {
                    is_eof_ = (ec == ppbox::demux::error::no_more_sample);
                    return false;
                }
            }
            //std::cout << "[SegmentFilter::get_sample] sample.ustime = " << sample.ustime << " segent_end_time_ = " << segent_end_time_ << std::endl;
            if (sample.ustime >= segent_end_time_
                && (video_track_ == boost::uint32_t(-1)
                    || (sample.itrack == video_track_
                    && (sample.flags & Sample::sync)))) {
                        ec = error::end_of_stream;
                        is_save_sample_ = true;
                        sample_ = sample;
                        return false;
            }
            return true;
        }

        bool SegmentFilter::before_seek(
            Sample & sample, 
            boost::system::error_code & ec)
        {
            is_save_sample_ = false;
            is_eof_ = false;
            sample.append(sample_);
            return Filter::before_seek(sample, ec);
        }

        void SegmentFilter::set_end_time(
            boost::uint64_t time)
        {
            segent_end_time_ = time;
        }

    } // namespace mux
} // namespace ppbox
