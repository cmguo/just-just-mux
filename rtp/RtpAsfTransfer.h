// RtpAsfTransfer.h

#ifndef _PPBOX_MUX_RTP_ASF_TRANSFER_H_
#define _PPBOX_MUX_RTP_ASF_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"

namespace ppbox
{
    namespace mux
    {

        class RtpAsfTransfer
            : public RtpTransfer
        {
        public:
            RtpAsfTransfer(
                MuxerBase & muxer);

            ~RtpAsfTransfer();

            virtual void transfer(
                ppbox::demux::Sample & sample);

            virtual void transfer(
                MediaInfoEx & info);

            void get_sdp(
                ppbox::demux::Sample const & tag, 
                std::string & sdp);

        private:
            boost::uint8_t header_[2][4];
        };
    }
}

#endif // _PPBOX_MUX_RTP_ASF_TRANSFER_H_
