// RtpAsfMux.h

#ifndef _PPBOX_MUX_RTP_ASF_MUX_H_
#define _PPBOX_MUX_RTP_ASF_MUX_H_

#include "ppbox/mux/asf/AsfMux.h"
#include "ppbox/mux/rtp/RtpMux.h"

namespace ppbox
{
    namespace mux
    {

        class RtpAsfTransfer;

        class RtpAsfMux
            : public RtpMux
        {
        public:
            RtpAsfMux();

            ~RtpAsfMux();

        public:
            void add_stream(
                MediaInfoEx & mediainfo);

            boost::system::error_code get_sdp(
                std::string & sdp_out,
                boost::system::error_code & ec);

        private:
            AsfMux asf_mux_;
            RtpAsfTransfer * rtp_asf_transfer_;
        };
    }
}

#endif // _PPBOX_MUX_RTP_ASF_MUX_H_
