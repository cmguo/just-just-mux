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
            SegmentFilter();

            virtual ~SegmentFilter();

            virtual boost::system::error_code open(
                MediaFileInfo const & media_file_info, 
                boost::system::error_code & ec);

            virtual boost::system::error_code get_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

            void set_end_time(
                boost::uint64_t time);

            void reset();

            bool is_sequence()
            {
                return is_save_sample_;
            }

        private:
            boost::uint32_t video_track_;
            boost::uint64_t segent_end_time_;
            boost::uint64_t fisrt_idr_timestamp_us_;
            bool is_save_sample_;
            ppbox::demux::Sample sample_;
        };
    }
}

#endif // End of _PPBOX_MUX_FILTER_SEGMENT_FILTER_H_
