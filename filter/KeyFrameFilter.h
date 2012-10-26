// KeyFrameFilter.h

#ifndef _PPBOX_MUX_FILTER_KEY_FRAME_FILTER_H_
#define _PPBOX_MUX_FILTER_KEY_FRAME_FILTER_H_

#include "ppbox/mux/Filter.h"

namespace ppbox
{
    namespace mux
    {

        class KeyFrameFilter
            : public Filter
        {
        public:
            KeyFrameFilter();

        public:
            virtual boost::system::error_code open(
                MediaInfo const & media_info, 
                std::vector<StreamInfo> const & streams, 
                boost::system::error_code & ec);

            virtual boost::system::error_code get_sample(
                Sample & sample,
                boost::system::error_code & ec);

        private:
            boost::uint32_t video_track_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FILTER_KEY_FRAME_FILTER_H_
