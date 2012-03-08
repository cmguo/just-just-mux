// RtpTsMux.h

#ifndef _PPBOX_MUX_RTP_TS_MUX_H_
#define _PPBOX_MUX_RTP_TS_MUX_H_

#include "ppbox/mux/ts/TsMux.h"
#include "ppbox/mux/rtp/RtpMux.h"

namespace ppbox
{
    namespace mux
    {

        class RtpTsTransfer;

        class RtpTsMux
            : public RtpMux
        {
        public:
            RtpTsMux();

            ~RtpTsMux();

        public:
            void add_stream(
                MediaInfoEx & mediainfo);

            void file_header(
                ppbox::demux::Sample & tag);

        private:
            TsMux ts_mux_;
            RtpTsTransfer * rtp_ts_transfer_;
        };

    }
}

#endif // _PPBOX_MUX_RTP_TS_MUX_H_
