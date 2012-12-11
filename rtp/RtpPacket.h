// RtpPacket.h

#ifndef _PPBOX_MUX_RTP_RTP_PACKET_H_
#define _PPBOX_MUX_RTP_RTP_PACKET_H_

#include "ppbox/mux/Transfer.h"

namespace ppbox
{
    namespace mux
    {

        class RtpTransfer;

        struct RtpHead
        {
            boost::uint8_t vpxcc;
            boost::uint8_t mpt;
            boost::uint16_t sequence;
            boost::uint32_t timestamp;
            boost::uint32_t ssrc;
        };

        struct RtpPacket
            : RtpHead
        {
            size_t size;
            size_t buf_beg;
            size_t buf_end;
        };

        struct RtpInfo
        {
            boost::uint32_t stream_index;
            boost::uint32_t timestamp;
            boost::uint32_t seek_time; // ms
            boost::uint32_t ssrc;
            boost::uint16_t sequence;
            std::string sdp;
            bool setup;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_PACKET_H_
