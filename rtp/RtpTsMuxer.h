// RtpTsMuxer.h

#ifndef _PPBOX_MUX_RTP_RTP_TS_MUXER_H_
#define _PPBOX_MUX_RTP_RTP_TS_MUXER_H_

#include "ppbox/mux/ts/TsMuxer.h"
#include "ppbox/mux/rtp/RtpMuxer.h"

namespace ppbox
{
    namespace mux
    {

        class RtpTsTransfer;

        class RtpTsMuxer
            : public RtpMuxer
        {
        public:
            RtpTsMuxer();

            ~RtpTsMuxer();

        public:
            void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);

            void file_header(
                Sample & sample);

        private:
            TsMuxer ts_mux_;
            RtpTsTransfer * rtp_ts_transfer_;
        };

        PPBOX_REGISTER_MUXER(rtp_ts, RtpTsMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_TS_MUXER_H_
