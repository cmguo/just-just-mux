// RtpAsfTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/asf/AsfTransfer.h"
#include "ppbox/mux/rtp/RtpAsfTransfer.h"

using namespace ppbox::avformat;

#include <framework/string/Format.h>
#include <framework/string/Base64.h>

#include <util/buffers/BuffersCopy.h>

namespace ppbox
{
    namespace mux
    {

        RtpAsfTransfer::RtpAsfTransfer(
            MuxerBase & muxer)
            : RtpTransfer(muxer, "RtpAsf", 96)
        {
            header_[0][0] = 0x40; // S = 0, L = 1, R = 0, D = 0. I = 0
            header_[0][1] = header_[0][2] = header_[0][3] = 0; // Length = 0
            header_[1][0] = 0x40; // S = 1, L = 1, R = 0, D = 0. I = 0
            header_[1][1] = header_[1][2] = header_[1][3] = 0; // Length = 0

            muxer.config().set("Asf", "packet_length", "1024");
        }

        RtpAsfTransfer::~RtpAsfTransfer()
        {
        }

        void RtpAsfTransfer::transfer(
            StreamInfo & info)
        {
            std::string sdp;
            if (info.type == MEDIA_TYPE_VIDE) {
                sdp = "m=video 0 RTP/AVP 96\r\n";
            } else {
                sdp = "m=audio 0 RTP/AVP 96\r\n";
            }
            sdp += "a=rtpmap:96 x-asf-pf/1000\r\n";
            sdp += "a=stream:" 
                + framework::string::format(info.index + 1)
                + "\r\n";
            sdp += "a=control:track"
                + framework::string::format(info.index)
                + "\r\n";
            rtp_info_.sdp += sdp;

            scale_.reset(1000, 1000);
        }

        void RtpAsfTransfer::transfer(
            Sample & sample)
        {
            RtpTransfer::begin(sample);
            std::vector<AsfTransfer::AsfPacket> const & packets = 
                *(std::vector<AsfTransfer::AsfPacket> const *)sample.context;
            std::deque<boost::asio::const_buffer>::const_iterator buf_beg = sample.data.begin();
            std::deque<boost::asio::const_buffer>::const_iterator buf_end = sample.data.end();
            for (size_t i = 0; i + 1 < packets.size(); ++i) { // i + 1, off_segs里面多记录了一个不完整packet的开始位置
                buf_end = sample.data.begin() + packets[i + 1].off_seg; // 最高位是关键帧标志
                RtpPacket p(sample.time, true);
                p.push_buffers(boost::asio::buffer(header_[packets[i].key_frame >> 31], 4));
                p.push_buffers(buf_beg, buf_end);
                p.size = 1024 + 4;
                push_packet(p);
                buf_beg = buf_end;
            }
            RtpTransfer::finish(sample);
        }

        void RtpAsfTransfer::get_sdp(
            Sample const & tag, 
            std::string & sdp)
        {
            std::string asf_head;
            asf_head.resize(util::buffers::buffers_size(tag.data));
            util::buffers::buffers_copy(boost::asio::buffer(&asf_head[0], asf_head.size()), tag.data);
            sdp += "a=maxps:1024\r\n";
            sdp += "a=pgmpu:data:application/vnd.ms.wms-hdr.asf1;base64," 
                + framework::string::Base64::encode(asf_head) 
                + "\r\n";
            sdp += rtp_info_.sdp;
        }

    } // namespace mux
} // namespace ppbox
