// RtpTransfer.h

#ifndef _PPBOX_MUX_RTP_RTP_TRANSFER_H_
#define _PPBOX_MUX_RTP_RTP_TRANSFER_H_

#include "ppbox/mux/rtp/RtpPacket.h"
#include "ppbox/mux/Transfer.h"

#include <framework/system/BytesOrder.h>
#include <framework/system/ScaleTransform.h>

namespace ppbox
{
    namespace mux
    {

        class RtpTransfer
            : public Transfer
        {
        public:
            RtpTransfer(
                MuxerBase & muxer, 
                std::string const & name, 
                boost::uint8_t type);

            virtual ~RtpTransfer();

        public:
            RtpInfo const & rtp_info() const
            {
                return rtp_info_;
            }

        public:
            virtual void on_seek(
                boost::uint64_t time);

            virtual void setup();

        protected:
            void begin(
                Sample & sample);

            void push_packet(
                RtpPacket & packet);

            void finish(
                Sample & sample);

        protected:
            RtpHead rtp_head_;
            RtpInfo rtp_info_;
            RtpSplitContent packets_;
            framework::system::ScaleTransform scale_; 
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_TRANSFER_H_
