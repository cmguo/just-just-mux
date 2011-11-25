// MuxerBase.h
#ifndef _PPBOX_MUX_MUXERBASER_H_
#define _PPBOX_MUX_MUXERBASER_H_

#include <ppbox/demux/base/DemuxerBase.h>
#include <boost/asio/buffer.hpp>

namespace ppbox
{

    namespace mux
    {

        struct MediaFileInfo
        {
            MediaFileInfo()
                : duration(0)
                , stream_count(0)
                , video_index(boost::uint32_t(-1))
                , audio_index(boost::uint32_t(-1))
                , attachment(NULL)
            {
            }

            boost::uint32_t duration;
            boost::uint32_t stream_count;
            boost::uint32_t video_index;
            boost::uint32_t audio_index;
            void * attachment;
            std::vector<ppbox::demux::MediaInfo> stream_infos;
        };

    }
}

#endif // _PPBOX_MUX_MUXERBASER_H_
