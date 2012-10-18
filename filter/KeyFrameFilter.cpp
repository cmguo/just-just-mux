// KeyFrameFilter.h

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"

using namespace ppbox::avformat;

#include <boost/asio/error.hpp>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        KeyFrameFilter::KeyFrameFilter()
            : video_track_(boost::uint32_t(-1))
        {
        }

        error_code KeyFrameFilter::open(
            MediaStreamInfo const & media_info, 
            boost::system::error_code & ec)
        {
            if (Filter::open(media_info, ec))
                return ec;
            video_track_ = boost::uint32_t(-1);
            for (size_t i = 0; i < media_info.streams.size(); ++i) {
                if (media_info.streams[i].type == MEDIA_TYPE_VIDE) {
                    video_track_ = i;
                    break;
                }
            }
            return ec; 
        }

        error_code KeyFrameFilter::get_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            Filter::get_sample(sample, ec);
            if (ec)
                return ec;
            if (video_track_ == boost::uint32_t(-1) 
                || (sample.itrack == video_track_
                && (sample.flags & Sample::sync))) {
                    detach_self();
                    return ec;
            }
            return boost::asio::error::would_block; 
        }

    } // namespace mux
} // namespace ppbox
