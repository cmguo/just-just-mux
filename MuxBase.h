// MuxBase.h

#ifndef _PPBOX_MUX_MUX_BASE_H_
#define _PPBOX_MUX_MUX_BASE_H_

#include "ppbox/avformat/Format.h"
#include "ppbox/avformat/codec/Codec.h"

#include <ppbox/data/MediaInfo.h>
#include <ppbox/data/PlayInfo.h>

namespace ppbox
{

    namespace mux
    {

        using ppbox::data::MediaInfo;
        using ppbox::data::PlayInfo;
        using ppbox::avformat::Sample;
        using ppbox::avformat::StreamInfo;

    }
}

#endif // _PPBOX_MUX_MUX_BASE_H_
