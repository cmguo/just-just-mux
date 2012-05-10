// SegmentFilter.h

#ifndef   _PPBOX_MUX_FILTER_SEGMENT_FILTER_H_
#define   _PPBOX_MUX_FILTER_SEGMENT_FILTER_H_

#include "ppbox/mux/filter/Filter.h"

namespace ppbox
{
    namespace mux
    {
        class SegmentFilter
            : public Filter
        {
        public:
            SegmentFilter(
                MediaFileInfo const & media_file_info)
                : Filter(media_file_info)
                , segent_end_time_(0)
                , fisrt_idr_timestamp_us_(boost::uint64_t(-1))
                , is_save_sample_(false)
            {
            }

            virtual ~SegmentFilter()
            {
            }

            boost::system::error_code get_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

            boost::uint64_t & segment_end_time(void)
            {
                return segent_end_time_;
            }

        private:
            boost::uint64_t segent_end_time_;
            boost::uint64_t fisrt_idr_timestamp_us_;
            bool is_save_sample_;
            ppbox::demux::Sample sample_;
        };
    }
}

#endif // End of _PPBOX_MUX_FILTER_SEGMENT_FILTER_H_
