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
                boost::uint8_t type)
            {
                static size_t g_ssrc = 0;
                if (g_ssrc == 0) {
                    g_ssrc = rand();
                }
                rtp_head_.vpxcc = 0x80;
                rtp_head_.mpt = type;
                rtp_head_.sequence = rand();
                rtp_head_.timestamp = rand();
                rtp_head_.ssrc = framework::system::BytesOrder::host_to_big_endian(g_ssrc++);
            }

            virtual ~RtpTransfer()
            {
            }

            virtual void get_rtp_info(
                MediaInfoEx & info)
            {
            }

            virtual void on_seek(
                boost::uint32_t time, 
                boost::uint32_t play_time)
            {
            }

            void clear(
                boost::uint64_t cts_ustime)
            {
                packets_.clear();
                packets_.ustime = cts_ustime;
            }

        protected:
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

            RtpSplitContent const & rtp_packets(void) const
            {
                return packets_;
            }

            RtpInfo & rep_info(void)
            {
                return rtp_info_;
            }

        protected:
            RtpHead rtp_head_;
            RtpInfo rtp_info_;
            RtpSplitContent packets_;
        };
    }
}

#endif // _PPBOX_MUX_RTP_ES_TRANSFER_H_
