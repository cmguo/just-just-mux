// RtpEsAudioTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/rtp/RtpEsAudioTransfer.h"

#include <framework/string/Base16.h>

namespace ppbox
{
    namespace mux
    {

        RtpEsAudioTransfer::RtpEsAudioTransfer(
            Muxer & muxer)
            : RtpTransfer(muxer, "RtpESAudio", 97)
            , index_(0)
            , time_adjust_(0)
        {
            au_header_section_[0] = 0;
            au_header_section_[1] = 16;
        }

        RtpEsAudioTransfer::~RtpEsAudioTransfer()
        {
        }

        void RtpEsAudioTransfer::transfer(
            MediaInfoEx & media)
        {
            using namespace framework::string;

            boost::uint32_t rtp_time_scale = media.time_scale;
            std::cout << "time_scale = " << media.time_scale << " sample_rate = " << media.audio_format.sample_rate << std::endl;
            if (media.time_scale < media.audio_format.sample_rate) {
                scale_.reset(media.audio_format.sample_rate, media.audio_format.sample_rate);
                rtp_time_scale = media.audio_format.sample_rate;
                time_adjust_ = 1;
            } else {
                scale_.reset(media.time_scale, media.time_scale);
            }
           
            std::string map_id_str = format(rtp_head_.mpt);
            rtp_info_.sdp = "m=audio 0 RTP/AVP " + map_id_str + "\r\n";
            rtp_info_.sdp += "a=rtpmap:" + map_id_str + " mpeg4-generic/" 
                + format(rtp_time_scale)
                + "/" + format(media.audio_format.channel_count) + "\r\n";
            rtp_info_.sdp += "a=fmtp:" + map_id_str 
                + " streamType=5"
                + ";profile-level-id=41"
                //+ ";objecttype=64"
                + ";mode=AAC-hbr"
                + ";sizeLength=13"
                + ";indexLength=3"
                + ";indexDeltaLength=3"
                + ";config=" + Base16::encode(std::string((char const *)&media.format_data.at(0), media.format_data.size()))
                + "\r\n";
            rtp_info_.sdp += "a=control:track" + format(media.index) + "\r\n";

            rtp_info_.stream_index = media.index;
        }

        void RtpEsAudioTransfer::transfer(
            ppbox::demux::Sample & sample)
        {
            au_header_section_[2] =  (boost::uint8_t)(sample.size >> 5);
            au_header_section_[3] = (boost::uint8_t)((sample.size << 3) /*| (index_++ & 0x07)*/);

            //std::cout << "audio dts = " << sample.dts << std::endl;
            //std::cout << "audio cts_delta = " << sample.cts_delta << std::endl;
            //std::cout << "audio cts = " << sample.dts + sample.cts_delta << std::endl;

            if (time_adjust_ == 0) {
                sample.dts = scale_.transfer(sample.dts);
            } else if (time_adjust_ == 1) {
                MediaInfoEx const & media = 
                    *(MediaInfoEx const *)sample.media_info;
                sample.dts = scale_.static_transfer(media.time_scale, media.audio_format.sample_rate, sample.dts);
                scale_.set(sample.dts);
                time_adjust_ = 2;
            } else {
                //MediaInfoEx const & media = 
                //    *(MediaInfoEx const *)sample.media_info;
                //boost::uint64_t dts2 = scale_.static_transfer(media.time_scale, media.audio_format.sample_rate, sample.dts);
                sample.dts = scale_.inc(1024);
                //std::cout << "audio dts = " << sample.dts << ", dts2 = " << dts2 << std::endl;
            }
            //std::cout << "sample track = " << sample.itrack << ", dts = " << dts << ", cts = " << cts << std::endl;
            RtpTransfer::clear(sample.ustime);
            RtpPacket packet(sample.dts, true);
            packet.size = sample.size + 4;
            packet.push_buffers(boost::asio::buffer(au_header_section_, 4));
            packet.push_buffers(sample.data);
            push_packet(packet);
            sample.context = (void*)&packets_;
        }

        void RtpEsAudioTransfer::on_seek(
            boost::uint32_t time)
        {
            RtpTransfer::on_seek(time);
            if (time_adjust_ == 2)
                time_adjust_ = 1;
        }

    }
}
