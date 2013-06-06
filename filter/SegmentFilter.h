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
            virtual bool put(
                StreamInfo & stream, 
                boost::system::error_code & ec);

            virtual bool put(
                Sample & sample,
                boost::system::error_code & ec);

            virtual bool put(
                MuxEvent const & event, 
                boost::system::error_code & ec);

        public:
            // time ��λ������
            void set_end_time(
                boost::uint64_t time);

            bool is_eof() const
            {
                return is_eof_;
            }

        private:
            boost::uint32_t video_track_;
            bool is_eof_;
            boost::uint64_t segent_end_time_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FILTER_SEGMENT_FILTER_H_
