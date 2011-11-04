// RtpPacket.h

#ifndef _PPBOX_MUX_RTP_PACKET_H_
#define _PPBOX_MUX_RTP_PACKET_H_

#include "ppbox/mux/transfer/Transfer.h"
//#include "ppbox/mux/transfer/RtpTransfer.h"

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

        //static boost::uint64_t TimescaleConvertTime(
        //    boost::uint64_t time_value,
        //    boost::uint32_t from_time_scale,
        //    boost::uint32_t to_time_scale)
        //{
        //    if (from_time_scale == 0) return 0;
        //    double ratio = (double)to_time_scale/(double)from_time_scale;
        //    return ((boost::uint64_t)((double)time_value*ratio));
        //}

        class RtpPacket
            : public RtpHead

        {
        public:
            RtpPacket(
                boost::uint32_t time,
                bool mark)
            {
                timestamp = time;
                mpt = mark ? 0x80 : 0;
                push_buffers(boost::asio::buffer(this, sizeof(RtpHead)));
            }

            RtpPacket(
                RtpPacket const & r)
                : RtpHead(r)
            {
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

            std::vector<boost::asio::const_buffer> buffers;
        };

        struct RtpSplitContent
        {
            std::vector<RtpPacket> packets;
        };

        struct RtpInfo
        {
            boost::uint32_t stream_index;
            boost::uint32_t timestamp;
            boost::uint32_t seek_time; // ms
            boost::uint32_t ssrc;
            boost::uint16_t sequence;
            std::string sdp;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_PACKET_H_
