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
                Muxer & muxer);

            ~RtpTsTransfer();

            virtual void transfer(
                MediaInfoEx & info);

            virtual void transfer(
                ppbox::demux::Sample & sample);

            void header_rtp_packet(
                ppbox::demux::Sample & tag);
        };
    }
}

#endif // _PPBOX_MUX_RTP_TS_TRANSFER_H_
