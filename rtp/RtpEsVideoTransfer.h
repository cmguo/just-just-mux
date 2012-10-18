// RtpEsVideoTransfer.h

#ifndef _PPBOX_MUX_RTP_RTP_ES_VIDEO_TRANSFER_H_
#define _PPBOX_MUX_RTP_RTP_ES_VIDEO_TRANSFER_H_

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
                MuxerBase & muxer);

            ~RtpEsVideoTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            boost::uint32_t mtu_size_;
            boost::uint8_t prefix_[3][2];
            boost::uint32_t sample_description_index_;
            boost::uint32_t use_dts_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_ES_VIDEO_TRANSFER_H_
