// RtpEsVideoTransfer.h

#ifndef _PPBOX_MUX_RTP_ES_VIDEO_TRANSFER_H_
#define _PPBOX_MUX_RTP_ES_VIDEO_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"

namespace ppbox
{
    namespace mux
    {

        class RtpEsVideoTransfer
            : public RtpTransfer
        {
        public:
            RtpEsVideoTransfer(
                Muxer & muxer);

            ~RtpEsVideoTransfer();

            virtual void transfer(
                MediaInfoEx & info);

            virtual void transfer(
                ppbox::demux::Sample & sample);

        private:
            boost::uint32_t mtu_size_;
            boost::uint8_t prefix_[3][2];
            boost::uint32_t sample_description_index_;
            boost::uint32_t use_dts_;
        };
    }
}
#endif // _PPBOX_MUX_RTP_ES_VIDEO_TRANSFER_H_
