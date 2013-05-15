// MuxBase.h

#ifndef _PPBOX_MUX_MUX_BASE_H_
#define _PPBOX_MUX_MUX_BASE_H_

#include <ppbox/demux/base/DemuxerBase.h>

#include <ppbox/data/base/MediaInfo.h>
#include <ppbox/data/base/StreamStatus.h>

namespace ppbox
{
    namespace mux
    {

        using ppbox::data::MediaInfo;
        using ppbox::data::StreamStatus;

        using ppbox::avbase::Sample;
        using ppbox::avbase::StreamInfo;

        class MuxerBase;

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MUX_BASE_H_
