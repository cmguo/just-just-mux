// RtpTsTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/ts/TsTransfer.h"
#include "ppbox/mux/rtp/RtpTsTransfer.h"
#include "ppbox/mux/rtp/RtpPacket.h"

const boost::uint32_t TS_PACKETS_PER_RTP_PACKET       = 7;

namespace ppbox
{
    namespace mux
    {

        RtpTsTransfer::RtpTsTransfer(
            MuxerBase & muxer)
            : RtpTransfer(muxer, "RtpTs", 33)
        {
        }

        RtpTsTransfer::~RtpTsTransfer()
        {
        }

        void RtpTsTransfer::transfer(
            StreamInfo & info)
        {
            rtp_info_.sdp = "m=video 0 RTP/AVP 33\r\n";
            rtp_info_.sdp += "a=rtpmap:33 MP2T/90000\r\n";
            rtp_info_.sdp += "a=control:track-1\r\n";

            scale_.reset(90000, 90000);
        }

        void RtpTsTransfer::transfer(
            Sample & sample)
        {
            RtpTransfer::clear(sample.ustime + sample.cts_delta * 1000000 / sample.stream_info->time_scale);
            std::vector<size_t> const & off_segs = 
                *(std::vector<size_t> const *)sample.context;
            std::deque<boost::asio::const_buffer>::const_iterator buf_beg = sample.data.begin();
            std::deque<boost::asio::const_buffer>::const_iterator buf_end = sample.data.end();
            boost::uint32_t i = TS_PACKETS_PER_RTP_PACKET;
            for (; i + 1 < off_segs.size(); i += TS_PACKETS_PER_RTP_PACKET) {
                // i + 1，这里+1是为了保证至少有一个RTP在后面生成，因为需要mark置为true
                buf_end = sample.data.begin() + off_segs[i];
                RtpPacket p(sample.dts + sample.cts_delta, false);
                p.size = TS_PACKETS_PER_RTP_PACKET * AP4_MPEG2TS_PACKET_SIZE;
                p.push_buffers(buf_beg, buf_end);
                push_packet(p);
                buf_beg = buf_end;
            }
            i -= TS_PACKETS_PER_RTP_PACKET;
            buf_end = sample.data.end();
            RtpPacket p(sample.dts + sample.cts_delta, true);
            p.size = (off_segs.size() - i) * AP4_MPEG2TS_PACKET_SIZE;
            p.push_buffers(buf_beg, buf_end);
            push_packet(p);

            sample.context = (void*)&packets_;
        }

        void RtpTsTransfer::header_rtp_packet(
            Sample & tag)
        {
            RtpTransfer::clear(0);
            RtpPacket p(rtp_info_.timestamp, true);
            p.push_buffers(tag.data);
            push_packet(p);
            tag.context = (void*)&packets_;
        }

    } // namespace mux
} // namespace ppbox
