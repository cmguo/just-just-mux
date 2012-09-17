// RtpEsMux.h

#ifndef _PPBOX_MUX_RTP_ES_MUX_H_
#define _PPBOX_MUX_RTP_ES_MUX_H_

#include "ppbox/mux/rtp/RtpMuxer.h"
#include <ppbox/demux/base/DemuxerBase.h>

namespace ppbox
{
    namespace mux
    {

        class RtpEsMuxer
            : public RtpMuxer
        {
        public:
            RtpEsMuxer();

            ~RtpEsMuxer();

        public:
            void add_stream(
                MediaInfoEx & mediainfo);
        };

        PPBOX_REGISTER_MUXER(rtp_es, RtpEsMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_ES_MUX_H_
