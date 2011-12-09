// MuxerBase.h
#ifndef _PPBOX_MUX_MUXERBASER_H_
#define _PPBOX_MUX_MUXERBASER_H_

#include "ppbox/mux/decode/Decode.h"
#include <ppbox/demux/base/DemuxerBase.h>

#include <boost/asio/buffer.hpp>

namespace ppbox
{

    namespace mux
    {

        struct MediaInfoEx
            : ppbox::demux::MediaInfo
        {
            MediaInfoEx()
                : MediaInfo()
                , decode(NULL)
                , config(NULL)
                , attachment(NULL)
            {
            }

            Decode * decode; // need delete
            void * config;
            void * attachment;
        };

        struct MediaFileInfo
        {
            MediaFileInfo()
                : duration(0)
                , stream_count(0)
                , attachment(NULL)
            {
            }

            boost::uint32_t duration;
            boost::uint32_t stream_count;
            void * attachment;
            std::vector<MediaInfoEx> stream_infos;
        };

    }
}

#endif // _PPBOX_MUX_MUXERBASER_H_
