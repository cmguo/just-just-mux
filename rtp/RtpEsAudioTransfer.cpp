// RtpEsAudioTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpEsAudioTransfer.h"

#include <framework/system/BytesOrder.h>

namespace ppbox
{
    namespace mux
    {
        class InterBitsReader
        {
        public:
            InterBitsReader(
                boost::uint8_t const * buf, 
                boost::uint32_t size)
                : buf_(buf)
                , size_(size - 1)
                , size_this_byte_(8)
                , failed_(false)
            {
                assert(size > 0);
            }

            boost::uint32_t read_bits(
                boost::uint32_t len)
            {
                if (size_this_byte_ + size_ * 8 < len || len > 32) {
                    failed_ = true;
                }
                if (failed_) {
                    return 0;
                }
                boost::uint32_t v = 0;
                while (len > size_this_byte_) {
                    v = (v << size_this_byte_) | (((boost::uint32_t)(*buf_)) & ((1 << size_this_byte_) - 1));
                    len -= size_this_byte_;
                    ++buf_;
                    --size_;
                    size_this_byte_ = 8;
                }
                if (len) {
                    v = (v << len) | ((((boost::uint32_t)(*buf_)) >> (size_this_byte_ - len)) & ((1 << len) - 1));
                    size_this_byte_ -= len;
                }
                return v;
            }

        private:
            boost::uint8_t const * buf_;
            boost::uint32_t size_;
            boost::uint32_t size_this_byte_;
            bool failed_;
        };

        RtpEsAudioTransfer::RtpEsAudioTransfer(
            Muxer & muxer,
            boost::uint8_t type)
            : RtpTransfer(type)
        {
            au_header_section_[0] = 0;
            au_header_section_[1] = 16;
            muxer.Config().register_module("RtpESAudio")
                << CONFIG_PARAM_NAME_RDWR("sequence", rtp_head_.sequence)
                << CONFIG_PARAM_NAME_RDWR("timestamp", rtp_head_.timestamp)
                << CONFIG_PARAM_NAME_RDWR("ssrc", rtp_head_.ssrc);
        }

        RtpEsAudioTransfer::~RtpEsAudioTransfer()
        {
        }


        void RtpEsAudioTransfer::transfer(ppbox::demux::Sample & sample)
        {
            RtpTransfer::clear();
            boost::uint32_t buffer_length = util::buffers::buffer_size(sample.data);
            au_header_section_[2] =  (boost::uint8_t)(buffer_length >> 5);
            au_header_section_[3] = (boost::uint8_t)((buffer_length << 3) | (index_++ & 0x07));

            RtpPacket packet(
                sample.dts + sample.cts_delta, 
                true);
            packet.push_buffers(boost::asio::buffer(au_header_section_, 4));
            packet.push_buffers(sample.data);
            push_packet(packet);
            sample.context = (void*)&rtp_packets();
        }

        void RtpEsAudioTransfer::get_rtp_info(ppbox::demux::MediaInfo & info)
        {
            using namespace framework::string;
            std::string map_id_str = format(rtp_head_.mpt);

            //InterBitsReader reader(&info.format_data.at(0), info.format_data.size());
            //boost::uint8_t object_type = (boost::uint8_t)reader.read_bits(5);

            rep_info().sdp = "m=audio 0 RTP/AVP " + map_id_str + "\r\n";
            rep_info().sdp += "a=rtpmap:" + map_id_str + " mpeg4-generic/" 
                + format(info.time_scale) 
                + "/" + format(info.audio_format.channel_count) + "\r\n";
            rep_info().sdp += "a=fmtp:" + map_id_str 
                + " profile-level-id=41"
                + "; config=" + Base16::encode(std::string((char const *)&info.format_data.at(0), info.format_data.size()))
                + "; streamType=5"
                + "; objectType=64"
                + "; mode=AAC-hbr"
                + "; sizeLength=13"
                + "; indexLength=3"
                + "; indexDeltaLength=3"
                + "\r\n";
            rep_info().sdp += "a=control:index=" + format(info.index) + "\r\n";
            time_scale_ = info.time_scale;

            rep_info().stream_index = info.index;
            rep_info().timestamp = rtp_head_.timestamp;
            rep_info().seek_time = 0;
            rep_info().ssrc = rtp_head_.ssrc;
            rep_info().sequence = rtp_head_.sequence;

            info.attachment = (void*)&rep_info();
        }

        void RtpEsAudioTransfer::on_seek(boost::uint32_t time, boost::uint32_t play_time)
        {
            boost::uint32_t time_offset = boost::uint64_t(time) * time_scale_ / 1000;
            boost::uint32_t play_time_offset = boost::uint64_t(play_time+10000) * time_scale_ / 1000;
            rtp_head_.timestamp += play_time_offset;
            rep_info().timestamp = rtp_head_.timestamp + time_offset;
            rep_info().seek_time = time;
            rep_info().sequence = rtp_head_.sequence;
        }

    }
}
