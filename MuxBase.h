// MuxBase.h

#ifndef _PPBOX_MUX_MUX_BASE_H_
#define _PPBOX_MUX_MUX_BASE_H_

#include "ppbox/avformat/Format.h"
#include "ppbox/avformat/codec/Codec.h"

#include <ppbox/data/MediaBase.h>

namespace ppbox
{

    namespace mux
    {

        using ppbox::avformat::Sample;

        class Transfer;

        struct MediaInfo
            : ppbox::data::MediaInfo
        {
            MediaInfo()
                : attachment(NULL)
            {
            }

            void * attachment;
        };

        struct StreamInfo
            : ppbox::avformat::StreamInfo
        {
            std::vector<Transfer *> transfers;
        };

        struct MediaStreamInfo
            : MediaInfo
        {
            std::vector<StreamInfo> streams;
        };

    }
}

#endif // _PPBOX_MUX_MUX_BASE_H_
