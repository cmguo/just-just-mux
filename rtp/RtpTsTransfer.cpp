// RtpTsTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/rtp/RtpTsTransfer.h"
#include "ppbox/mux/rtp/RtpPacket.h"

#include <ppbox/avformat/ts/TsPacket.h>
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        static boost::uint32_t const TS_PACKETS_PER_RTP_PACKET = 7;

        RtpTsTransfer::RtpTsTransfer()
            : RtpTransfer("RtpTs", 33, TsPacket::TIME_SCALE)
        {
        }

        RtpTsTransfer::~RtpTsTransfer()
        {
        }

        void RtpTsTransfer::transfer(
            StreamInfo & info)
        {
            RtpTransfer::transfer(info);

            rtp_info_.sdp = "m=video 0 RTP/AVP 33\r\n";
            rtp_info_.sdp += "a=rtpmap:33 MP2T/90000\r\n";
            rtp_info_.sdp += "a=control:track-1\r\n";
        }

        void RtpTsTransfer::transfer(
            Sample & sample)
        {
            // Don't need adjust time scale, ts transfer already done it
            //RtpTransfer::transfer(sample);

            RtpTransfer::begin(sample);
            std::vector<size_t> const & off_segs = 
                *(std::vector<size_t> const *)sample.context;
            std::deque<boost::asio::const_buffer>::const_iterator buf_beg = sample.data.begin();
            std::deque<boost::asio::const_buffer>::const_iterator buf_end = sample.data.end();
            boost::uint32_t i = TS_PACKETS_PER_RTP_PACKET;
            for (; i + 1 < off_segs.size(); i += TS_PACKETS_PER_RTP_PACKET) {
                // i + 1，这里+1是为了保证至少有一个RTP在后面生成，因为需要mark置为true
                buf_end = sample.data.begin() + off_segs[i];
                begin_packet(false, sample.dts + sample.cts_delta, TS_PACKETS_PER_RTP_PACKET * TsPacket::PACKET_SIZE);
                push_buffers(buf_beg, buf_end);
                finish_packet();
                buf_beg = buf_end;
            }
            i -= TS_PACKETS_PER_RTP_PACKET;
            buf_end = sample.data.end();
            begin_packet(true, sample.dts + sample.cts_delta, (off_segs.size() - i) * TsPacket::PACKET_SIZE);
            push_buffers(buf_beg, buf_end);
            finish_packet();
            RtpTransfer::finish(sample);
        }

        void RtpTsTransfer::header_rtp_packet(
            Sample & sample)
        {
            RtpTransfer::begin(sample);
            begin_packet(true, rtp_info_.timestamp, sample.size);
            push_buffers(sample.data);
            finish_packet();
            RtpTransfer::finish(sample);
        }

    } // namespace mux
} // namespace ppbox
