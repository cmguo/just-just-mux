// KeyFrameFilter.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/filter/MergeFilter.h"
#include "ppbox/mux/MuxError.h"

namespace ppbox
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
                ec = error::need_more_sample;
                return false;
            }
        }

    } // namespace mux
} // namespace ppbox
