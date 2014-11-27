// KeyFrameFilter.cpp

#include "just/mux/Common.h"
#include "just/mux/filter/KeyFrameFilter.h"
#include "just/mux/filter/MergeFilter.h"
#include "just/mux/MuxError.h"

namespace just
{
    namespace mux
    {

        KeyFrameFilter::KeyFrameFilter()
            : video_track_(boost::uint32_t(-1))
        {
        }

        bool KeyFrameFilter::put(
            StreamInfo & info, 
            boost::system::error_code & ec)
        {
            if (info.type == StreamType::VIDE) {
                video_track_ = info.index;
            }
            return Filter::put(info, ec); 
        }

        bool KeyFrameFilter::put(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (video_track_ == boost::uint32_t(-1) 
                || (sample.itrack == video_track_
                && (sample.flags & Sample::f_sync))) {
                    bool r = Filter::put(sample, ec);
                    MergeFilter::detach(this);
                    return r;
            } else {
                return true;
            }
        }

    } // namespace mux
} // namespace just
