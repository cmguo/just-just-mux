// KeyFrameFilter.h

#include "ppbox/mux/Common.h"
#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        error_code KeyFrameFilter::get_sample(
            ppbox::demux::Sample & sample,
            boost::system::error_code & ec)
        {
            while (true) {
                Filter::get_sample(sample, ec);
                if (ec)
                    break;
                if (sample.itrack == media_file_info().video_index && (sample.flags & demux::Sample::sync)) {
                    detach_self();
                    break;
                } else if (sample.itrack == media_file_info().audio_index) {
                    break;
                }
            }
            return ec;
        }

    }
}
