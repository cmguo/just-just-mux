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
                if (media_file_info().stream_infos[sample.itrack].type == ppbox::demux::MEDIA_TYPE_VIDE
                    && (sample.flags & demux::Sample::sync)) {
                    detach_self();
                    break;
                } else if (media_file_info().stream_infos[sample.itrack].type == ppbox::demux::MEDIA_TYPE_AUDI) {
                    break;
                }
            }
            return ec;
        }

    }
}
