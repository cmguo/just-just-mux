// RtpTsTransfer.h

#ifndef _PPBOX_MUX_RTP_TS_TRANSFER_H_
#define _PPBOX_MUX_RTP_TS_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"

namespace ppbox
{
    namespace mux
    {

        class RtpTsTransfer
            : public RtpTransfer
        {
        public:
            RtpTsTransfer(
                MuxerBase & muxer);

            ~RtpTsTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

            void header_rtp_packet(
                Sample & tag);
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_TS_TRANSFER_H_
