// SegmentFilter.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/filter/SegmentFilter.h"
#include "ppbox/mux/filter/MergeFilter.h"
#include "ppbox/mux/MuxError.h"

#include <ppbox/avbase/StreamType.h>
using namespace ppbox::avbase;

namespace ppbox
{
    namespace mux
    {

        SegmentFilter::SegmentFilter()
            : video_track_(boost::uint32_t(-1))
            , segent_end_time_(0)
            , is_eof_(false)
        {
        }

        SegmentFilter::~SegmentFilter()
        {
        }

        bool SegmentFilter::put(
            StreamInfo & info, 
            boost::system::error_code & ec)
        {
            if (info.type == StreamType::VIDE) {
                assert(video_track_ == boost::uint32_t(-1));
                video_track_ = info.index;
            }
            return Filter::put(info, ec);
        }

        bool SegmentFilter::put(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (sample.time >= segent_end_time_
                && (video_track_ == boost::uint32_t(-1)
                || (sample.itrack == video_track_
                && (sample.flags & Sample::f_sync)))) {
                    is_eof_ = true;
                    MergeFilter::put_eof(this, ec);
                    ec = error::end_of_stream;
                    return false;
            }
            return Filter::put(sample, ec);
        }

        bool SegmentFilter::put(
            Filter::eos_t & eos, 
            boost::system::error_code & ec)
        {
            is_eof_ = true;
            return Filter::put(eos, ec);
        }

        bool SegmentFilter::reset(
            Sample & sample, 
            boost::system::error_code & ec)
        {
            is_eof_ = false;
            return Filter::reset(sample, ec);
        }

        void SegmentFilter::set_end_time(
            boost::uint64_t time)
        {
            is_eof_ = false;
            segent_end_time_ = time;
        }

    } // namespace mux
} // namespace ppbox
