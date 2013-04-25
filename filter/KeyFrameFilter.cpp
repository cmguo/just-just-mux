// KeyFrameFilter.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"

#include <ppbox/avformat/Format.h>
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

        bool KeyFrameFilter::open(
            MediaInfo const & media_info, 
            std::vector<StreamInfo> const & streams, 
            boost::system::error_code & ec)
        {
            if (!Filter::open(media_info, streams, ec))
                return false;
            video_track_ = boost::uint32_t(-1);
            for (size_t i = 0; i < streams.size(); ++i) {
                if (streams[i].type == StreamType::VIDE) {
                    video_track_ = i;
                    break;
                }
            }
            return true; 
        }

        bool KeyFrameFilter::get_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            while (Filter::get_sample(sample, ec)) {
                if (video_track_ == boost::uint32_t(-1) 
                    || (sample.itrack == video_track_
                    && (sample.flags & Sample::f_sync))) {
                        detach_self();
                        return true;
                }
            }
            return false;
        }

    } // namespace mux
} // namespace ppbox
