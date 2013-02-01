// RtpAsfTransfer.h

#ifndef _PPBOX_MUX_RTP_RTP_ASF_TRANSFER_H_
#define _PPBOX_MUX_RTP_RTP_ASF_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"

namespace ppbox
{
    namespace mux
    {

        class RtpAsfTransfer
            : public RtpTransfer
        {
        public:
            RtpAsfTransfer();

            ~RtpAsfTransfer();

        public:
            virtual void transfer(
                Sample & sample);

            virtual void transfer(
                StreamInfo & info);

        public:
            void get_sdp(
                Sample const & tag, 
                std::string & sdp);

        private:
            boost::uint8_t header_[2][4];
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_ASF_TRANSFER_H_
