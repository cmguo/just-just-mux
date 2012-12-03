// RtpRawMuxer.h

#ifndef _PPBOX_MUX_RTP_RTP_RAW_MUXER_H_
#define _PPBOX_MUX_RTP_RTP_RAW_MUXER_H_

#include "ppbox/mux/rtp/RtpMuxer.h"

namespace ppbox
{
    namespace mux
    {

        class RtpRawMuxer
            : public RtpMuxer
        {
        public:
            RtpRawMuxer();

            ~RtpRawMuxer();

        public:
            void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);
        };

        PPBOX_REGISTER_MUXER("rtp-raw", RtpRawMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_RAW_MUXER_H_
