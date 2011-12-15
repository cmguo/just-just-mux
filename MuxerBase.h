// MuxerBase.h
#ifndef _PPBOX_MUX_MUXERBASER_H_
#define _PPBOX_MUX_MUXERBASER_H_

#include "ppbox/avformat/codec/Codec.h"
#include <ppbox/demux/base/DemuxerBase.h>

#include <boost/asio/buffer.hpp>

namespace ppbox
{

    namespace mux
    {

        class Transfer;

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

            ppbox::avformat::Codec * decode;
            void * config;
            void * attachment;
            std::vector<Transfer *> transfers;
        };

        struct MediaFileInfo
        {
            MediaFileInfo()
                : duration(0)
                , stream_count(0)
                , filesize(0)
                , attachment(NULL)
            {
            }

            boost::uint32_t duration;
            boost::uint32_t stream_count;
            boost::uint32_t filesize;
            void * attachment;
            std::vector<MediaInfoEx> stream_infos;
        };

    }
}

#endif // _PPBOX_MUX_MUXERBASER_H_
