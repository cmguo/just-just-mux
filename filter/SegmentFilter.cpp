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
            , is_eof_(false)
            , segent_end_time_(0)
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
            MuxEvent const & event, 
            boost::system::error_code & ec)
        {
            switch (event.type) {
                case MuxEvent::end:
                    is_eof_ = true;
                    break;
                case MuxEvent::reset:
                    is_eof_ = false;
                    break;
                default:
                    break;
            }
            return Filter::put(event, ec);
        }

        void SegmentFilter::set_end_time(
            boost::uint64_t time)
        {
            is_eof_ = false;
            segent_end_time_ = time;
        }

    } // namespace mux
} // namespace ppbox
