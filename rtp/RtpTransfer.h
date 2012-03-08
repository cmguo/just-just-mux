// RtpTransfer.h

#ifndef   _PPBOX_MUX_RTP_RTP_TRANSFER_H_
#define   _PPBOX_MUX_RTP_RTP_TRANSFER_H_

#include "ppbox/mux/rtp/RtpPacket.h"
#include "ppbox/mux/transfer/Transfer.h"

#include <framework/system/BytesOrder.h>
#include <framework/string/Format.h>
#include <framework/string/Base16.h>
#include <framework/string/Base64.h>
using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        class RtpTransfer
            : public Transfer
        {
        public:
            RtpTransfer(
                Muxer & muxer, 
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
                boost::uint32_t time, 
                boost::uint32_t play_time);

            virtual void setup();

        protected:
            void clear(
                boost::uint64_t cts_ustime)
            {
                packets_.clear();
                packets_.ustime = cts_ustime;
            }

            void push_packet(
                RtpPacket & packet)
            {
                packet.vpxcc = rtp_head_.vpxcc;
                packet.mpt |= rtp_head_.mpt;
                packet.sequence = framework::system::BytesOrder::host_to_big_endian(rtp_head_.sequence++);
                packet.timestamp = framework::system::BytesOrder::host_to_big_endian(
                    rtp_head_.timestamp + packet.timestamp);
                packet.ssrc = rtp_head_.ssrc;
                packets_.push_back(packet);
            }

        protected:
            RtpHead rtp_head_;
            RtpInfo rtp_info_;
            RtpSplitContent packets_;
            boost::uint32_t time_scale_in_ms_; 
        };
    }
}

#endif // _PPBOX_MUX_RTP_ES_TRANSFER_H_
