// KeyFrameFilter.h

#ifndef   _PPBOX_MUX_KEY_FRAME_FILTER_H_
#define   _PPBOX_MUX_KEY_FRAME_FILTER_H_

#include "ppbox/mux/filter/Filter.h"

namespace ppbox
{
    namespace mux
    {
        class KeyFrameFilter
            : public Filter
        {
        public:
            virtual boost::system::error_code open(
                MediaFileInfo const & media_file_info, 
                boost::system::error_code & ec);

            virtual boost::system::error_code get_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

        private:
            boost::uint32_t video_track_;
        };
    }
}

#endif
