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
            KeyFrameFilter(
                MediaFileInfo const & media_file_info)
                : Filter(media_file_info)
            {
            }

            virtual ~KeyFrameFilter()
            {
            }

            boost::system::error_code get_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);
        };
    }
}

#endif
