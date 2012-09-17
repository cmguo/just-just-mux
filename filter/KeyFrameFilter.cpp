// KeyFrameFilter.h

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"

#include <boost/asio/error.hpp>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        error_code KeyFrameFilter::open(
            MediaFileInfo const & media_file_info, 
            boost::system::error_code & ec)
        {
            if (Filter::open(media_file_info, ec))
                return ec;
            video_track_ = boost::uint32_t(-1);
            for (size_t i = 0; i < media_file_info.stream_infos.size(); ++i) {
                if (media_file_info.stream_infos[i].type == ppbox::demux::MEDIA_TYPE_VIDE) {
                    video_track_ = i;
                    break;
                }
            }
            return ec; 
        }

        error_code KeyFrameFilter::get_sample(
            ppbox::demux::Sample & sample,
            boost::system::error_code & ec)
        {
            Filter::get_sample(sample, ec);
            if (ec)
                return ec;
            if (video_track_ == boost::uint32_t(-1) 
                || (sample.itrack == video_track_
                && (sample.flags & demux::Sample::sync))) {
                    detach_self();
                    return ec;
            }
            return boost::asio::error::would_block; 
        }

    }
}
