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

        class RtpPacket
            : public RtpHead

        {
        public:
            RtpPacket(
                bool mark, 
                boost::uint64_t time,
                boost::uint32_t size)
                : size(size + 4)
            {
                timestamp = (boost::uint32_t)time;
                mpt = mark ? 0x80 : 0;
                push_buffers(boost::asio::buffer(this, sizeof(RtpHead)));
            }

            RtpPacket(
                RtpPacket const & r)
                : RtpHead(r)
            {
                size = r.size;
                buffers = r.buffers;
                buffers[0] = boost::asio::buffer(this, sizeof(RtpHead));
            }

            template <typename ConstBuffers>
            void push_buffers(
                ConstBuffers const & buffers1)
            {
                buffers.insert(buffers.end(), buffers1.begin(), buffers1.end());
            }

            template <typename ConstBuffersIterator>
            void push_buffers(
                ConstBuffersIterator const & beg, 
                ConstBuffersIterator const & end)
            {
                buffers.insert(buffers.end(), beg, end);
            }

            size_t size;
            std::vector<boost::asio::const_buffer> buffers;
        };

        struct RtpSplitContent
            : std::vector<RtpPacket>
        {
            boost::uint64_t total_size;
            boost::uint64_t ustime; // cts
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
