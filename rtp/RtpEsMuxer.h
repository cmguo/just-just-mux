// RtpEsMux.h

#ifndef _PPBOX_MUX_RTP_RTP_ES_MUXER_H_
#define _PPBOX_MUX_RTP_RTP_ES_MUXER_H_

#include "ppbox/mux/rtp/RtpMuxer.h"

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
                StreamInfo & info);
        };

        PPBOX_REGISTER_MUXER(rtp_es, RtpEsMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_ES_MUXER_H_
