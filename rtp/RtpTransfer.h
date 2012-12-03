// RtpTransfer.h

#ifndef _PPBOX_MUX_RTP_RTP_TRANSFER_H_
#define _PPBOX_MUX_RTP_RTP_TRANSFER_H_

#include "ppbox/mux/rtp/RtpPacket.h"
#include "ppbox/mux/transfer/TimeScaleTransfer.h"

#include <framework/system/BytesOrder.h>
#include <framework/system/ScaleTransform.h>

namespace ppbox
{
    namespace mux
    {

        class RtpTransfer
            : public TimeScaleTransfer
        {
        public:
            RtpTransfer(
                char const * const name, 
                boost::uint32_t time_scale, 
                boost::uint8_t type);

            virtual ~RtpTransfer();

        public:
            virtual void config(
                framework::configure::Config & conf);

        public:
            virtual void on_seek(
                boost::uint64_t time);

            virtual void setup();

        public:
            RtpInfo const & rtp_info() const
            {
                return rtp_info_;
            }

        protected:
            void begin(
                Sample & sample);

            void push_packet(
                RtpPacket & packet);

            void finish(
                Sample & sample);

        protected:
            char const * const name_;
            RtpHead rtp_head_;
            RtpInfo rtp_info_;
            RtpSplitContent packets_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_TRANSFER_H_
