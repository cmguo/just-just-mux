// RtpAudioMpegTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpAudioMpegTransfer.h"

namespace ppbox
{
    namespace mux
    {

        RtpAudioMpegTransfer::RtpAudioMpegTransfer(
            Muxer & muxer,
            boost::uint8_t type)
            : RtpTransfer(type)
        {
            header_[0] = 0;
            header_[1] = 0;
            muxer.Config().register_module("RtpAudioMpegTransfer")
                << CONFIG_PARAM_NAME_RDWR("sequence", rtp_head_.sequence)
                << CONFIG_PARAM_NAME_RDWR("timestamp", rtp_head_.timestamp)
                << CONFIG_PARAM_NAME_RDWR("ssrc", rtp_head_.ssrc);
        }

        RtpAudioMpegTransfer::~RtpAudioMpegTransfer()
        {
        }

        void RtpAudioMpegTransfer::transfer(
            ppbox::demux::Sample & sample)
        {
            MediaInfoEx const * audio_info = (MediaInfoEx const *)sample.media_info;
            RtpTransfer::clear(sample.ustime);
            RtpPacket packet(
                (sample.dts + sample.cts_delta) * 90000 / audio_info->time_scale, 
                true);
            packet.size = sample.size + 4;
            packet.push_buffers(boost::asio::buffer(header_, 4));
            packet.push_buffers(sample.data);
            push_packet(packet);
            sample.context = (void*)&rtp_packets();
        }

        void RtpAudioMpegTransfer::get_rtp_info(
            MediaInfoEx & info)
        {
            using namespace framework::string;

            std::string map_id_str = format(rtp_head_.mpt);
            rep_info().sdp = "m=audio 0 RTP/AVP " + map_id_str + "\r\n";
            rep_info().sdp += "a=rtpmap:" + map_id_str + " mpa/90000" 
                + "/" + format(info.audio_format.channel_count)
                + "\r\n";
            rep_info().sdp += "a=control:index=" + format(info.index) + "\r\n";

            rep_info().stream_index = info.index;
            rep_info().timestamp = rtp_head_.timestamp;
            rep_info().seek_time = 0;
            rep_info().ssrc = rtp_head_.ssrc;
            rep_info().sequence = rtp_head_.sequence;

            info.attachment = (void*)&rep_info();
        }

        void RtpAudioMpegTransfer::on_seek(boost::uint32_t time, boost::uint32_t play_time)
        {
            boost::uint32_t time_offset = boost::uint64_t(time) * 90;
            boost::uint32_t play_time_offset = boost::uint64_t(play_time + 10000) * 90;
            rtp_head_.timestamp += play_time_offset;
            rep_info().timestamp = rtp_head_.timestamp + time_offset;
            rep_info().seek_time = time;
            rep_info().sequence = rtp_head_.sequence;
        }

    }
}
