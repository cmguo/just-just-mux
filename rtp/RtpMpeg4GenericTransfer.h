// RtpMpeg4GenericTransfer.h

#ifndef _PPBOX_MUX_RTP_RTP_MPEG4_GENERIC_TRANSFER_H_
#define _PPBOX_MUX_RTP_RTP_MPEG4_GENERIC_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"

namespace ppbox
{
    namespace mux
    {

        class RtpMpeg4GenericTransfer
            : public RtpTransfer
        {
        public:
            RtpMpeg4GenericTransfer();

            ~RtpMpeg4GenericTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            boost::uint8_t index_;
            boost::uint8_t au_header_section_[4];
            std::vector<boost::uint8_t> packat_header_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_MPEG4_GENERIC_TRANSFER_H_
