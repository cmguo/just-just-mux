// SegmentFilter.h

#ifndef _PPBOX_MUX_FILTER_SEGMENT_FILTER_H_
#define _PPBOX_MUX_FILTER_SEGMENT_FILTER_H_

#include "ppbox/mux/Filter.h"

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

        public:
            virtual bool open(
                MediaInfo const & media_info, 
                std::vector<StreamInfo> const & streams, 
                boost::system::error_code & ec);

            virtual bool get_sample(
                Sample & sample,
                boost::system::error_code & ec);

            virtual bool before_seek(
                Sample & sample, 
                boost::system::error_code & ec);

        public:
            // time µ•Œª£∫∫¡√Î
            void set_end_time(
                boost::uint64_t time);

            bool is_sequence()
            {
                return is_save_sample_ || is_eof_;
            }

        private:
            boost::uint32_t video_track_;
            boost::uint64_t segent_end_time_;
            bool is_save_sample_;
            bool is_eof_;
            Sample sample_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FILTER_SEGMENT_FILTER_H_
