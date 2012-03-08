// RtpEsAudioTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/Muxer.h"
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
            Muxer & muxer)
            : RtpTransfer(muxer, "RtpESAudio", 97)
        {
            au_header_section_[0] = 0;
            au_header_section_[1] = 16;
        }

        RtpEsAudioTransfer::~RtpEsAudioTransfer()
        {
        }

        void RtpEsAudioTransfer::transfer(
            MediaInfoEx & info)
        {
            using namespace framework::string;

            //InterBitsReader reader(&info.format_data.at(0), info.format_data.size());
            //boost::uint8_t object_type = (boost::uint8_t)reader.read_bits(5);

            boost::uint32_t rtp_time_scale = 48000;
            if ((rtp_time_scale % info.time_scale) == 0) {
                time_scale_ = rtp_time_scale / info.time_scale;
            } else {
                rtp_time_scale = info.time_scale;
                time_scale_ = 1;
            }
            time_scale_in_ms_ = rtp_time_scale / 1000;

            std::string map_id_str = format(rtp_head_.mpt);
            rtp_info_.sdp = "m=audio 0 RTP/AVP " + map_id_str + "\r\n";
            rtp_info_.sdp += "a=rtpmap:" + map_id_str + " mpeg4-generic/" 
                + format(rtp_time_scale)
                + "/" + format(info.audio_format.channel_count) + "\r\n";
            rtp_info_.sdp += "a=fmtp:" + map_id_str 
                + " streamType=5"
                + ";profile-level-id=41"
                //+ ";objecttype=64"
                + ";mode=AAC-hbr"
                + ";sizeLength=13"
                + ";indexLength=3"
                + ";indexDeltaLength=3"
                + ";config=" + Base16::encode(std::string((char const *)&info.format_data.at(0), info.format_data.size()))
                + "\r\n";
            rtp_info_.sdp += "a=control:index=" + format(info.index) + "\r\n";

            rtp_info_.stream_index = info.index;
        }

        void RtpEsAudioTransfer::transfer(
            ppbox::demux::Sample & sample)
        {
            au_header_section_[2] =  (boost::uint8_t)(sample.size >> 5);
            au_header_section_[3] = (boost::uint8_t)((sample.size << 3) /*| (index_++ & 0x07)*/);

            //std::cout << "audio dts = " << sample.dts << std::endl;
            //std::cout << "audio cts_delta = " << sample.cts_delta << std::endl;
            //std::cout << "audio cts = " << sample.dts + sample.cts_delta << std::endl;

            RtpTransfer::clear(sample.ustime);
            RtpPacket packet(
                (sample.dts + sample.cts_delta) * time_scale_, 
                true);
            packet.size = sample.size + 4;
            packet.push_buffers(boost::asio::buffer(au_header_section_, 4));
            packet.push_buffers(sample.data);
            push_packet(packet);
            sample.context = (void*)&packets_;
        }

    }
}
