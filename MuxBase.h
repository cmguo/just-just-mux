// MuxBase.h

#ifndef _PPBOX_MUX_MUX_BASE_H_
#define _PPBOX_MUX_MUX_BASE_H_

#include "ppbox/avformat/Format.h"
#include "ppbox/avformat/codec/Codec.h"

#include <ppbox/demux/base/DemuxerBase.h>

#include <ppbox/data/base/MediaInfo.h>
#include <ppbox/data/base/PlayInfo.h>

namespace ppbox
{
    namespace mux
    {

        using ppbox::data::MediaInfo;
        using ppbox::data::PlayInfo;

        using ppbox::avformat::Sample;
        using ppbox::avformat::StreamInfo;

        class MuxerBase;

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MUX_BASE_H_
