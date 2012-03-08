// RtpEsMux.h

#ifndef _PPBOX_MUX_RTP_ES_MUX_H_
#define _PPBOX_MUX_RTP_ES_MUX_H_

#include "ppbox/mux/rtp/RtpMux.h"
#include <ppbox/demux/base/DemuxerBase.h>

namespace ppbox
{
    namespace mux
    {

        class RtpEsMux
            : public RtpMux
        {
        public:
            RtpEsMux();

            ~RtpEsMux();

        public:
            void add_stream(
                MediaInfoEx & mediainfo);
        };
    }
}

#endif // _PPBOX_MUX_RTP_ES_MUX_H_
