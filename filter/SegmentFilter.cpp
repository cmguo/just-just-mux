// SegmentFilter.h

#include "ppbox/mux/Common.h"
#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/filter/SegmentFilter.h"

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {
        error_code SegmentFilter::get_sample(
            ppbox::demux::Sample & sample,
            boost::system::error_code & ec)
        {
            if (is_save_sample_) {
                sample = sample_;
                is_save_sample_ = false;
            } else {
                Filter::get_sample(sample, ec);
            }

            if (!ec) {
                if (fisrt_idr_timestamp_us_ == boost::uint64_t(-1)
                    && sample.itrack == media_file_info().video_index
                    && sample.is_sync) {
                        fisrt_idr_timestamp_us_ = sample.ustime;
                } else {
                    if (sample.itrack == media_file_info().video_index
                        && sample.is_sync) {
                            if ((sample.ustime - fisrt_idr_timestamp_us_) 
                                >= segent_end_time_ ) {
                                    ec = error::mux_segment_end;
                                    is_save_sample_ = true;
                                    sample_ = sample;
                            }
                    }
                }
            }
            return ec;
        }
    }
}