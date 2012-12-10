// RtpTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/rtp/RtpTransfer.h"

namespace ppbox
{
    namespace mux
    {

        RtpTransfer::RtpTransfer(
            char const * const name, 
            boost::uint8_t type, 
            boost::uint32_t time_scale)
            : TimeScaleTransfer(time_scale)
            , name_(name)
        {
            static size_t g_ssrc = 0;
            if (g_ssrc == 0) {
                g_ssrc = rand();
            }
            rtp_head_.vpxcc = 0x80;
            rtp_head_.mpt = type;
            rtp_head_.sequence = rand();
            rtp_head_.timestamp = rand();
            rtp_head_.ssrc = framework::system::BytesOrder::host_to_big_endian(g_ssrc++);

            rtp_info_.stream_index = (boost::uint32_t)-1;
            rtp_info_.timestamp = rtp_head_.timestamp;
            rtp_info_.seek_time = 0;
            rtp_info_.ssrc = rtp_head_.ssrc;
            rtp_info_.sequence = rtp_head_.sequence;
            rtp_info_.setup = false;

        }

        RtpTransfer::~RtpTransfer()
        {
        }

        void RtpTransfer::config(
            framework::configure::Config & conf)
        {
            conf.register_module(name_)
                << CONFIG_PARAM_NAME_RDWR("sequence", rtp_head_.sequence)
                << CONFIG_PARAM_NAME_RDWR("timestamp", rtp_head_.timestamp)
                << CONFIG_PARAM_NAME_RDWR("ssrc", rtp_head_.ssrc);
        }

        void RtpTransfer::setup()
        {
            rtp_info_.setup = true;
        }

        void RtpTransfer::on_seek(
            boost::uint64_t time)
        {
            if (packets_.empty())
                return;
            boost::uint32_t last_timestamp = 
                framework::system::BytesOrder::host_to_big_endian(packets_[0].timestamp);
            // std::cout << "last_timestamp = " << last_timestamp << std::endl;
            rtp_info_.timestamp = last_timestamp + (boost::uint32_t)scale_.scale_out() * 8; // add 8 seconds to keep distance from timestamp before
            // std::cout << "rtp_info_.timestamp = " << rtp_info_.timestamp << std::endl;
            rtp_head_.timestamp = rtp_info_.timestamp - (boost::uint32_t)scale_.static_transfer(1000, scale_.scale_out(), time);
            // std::cout << "rtp_head_.timestamp = " << rtp_head_.timestamp << std::endl;
            rtp_info_.sequence = rtp_head_.sequence;
            rtp_info_.seek_time = (boost::uint32_t)time;

            TimeScaleTransfer::on_seek(time);
        }

        void RtpTransfer::begin(
            Sample & sample)
        {
            packets_.clear();
            packets_.ustime = sample.ustime + sample.cts_delta * 1000000 / sample.stream_info->time_scale;
        }

        void RtpTransfer::push_packet(
            RtpPacket & packet)
        {
            packet.vpxcc = rtp_head_.vpxcc;
            packet.mpt |= rtp_head_.mpt;
            packet.sequence = framework::system::BytesOrder::host_to_big_endian(rtp_head_.sequence++);
            packet.timestamp = framework::system::BytesOrder::host_to_big_endian(
                rtp_head_.timestamp + packet.timestamp);
            packet.ssrc = rtp_head_.ssrc;
            packets_.push_back(packet);
        }

        void RtpTransfer::finish(
            Sample & sample)
        {
            sample.size = packets_.size();
            sample.data.clear();
            sample.data.push_back(boost::asio::const_buffer(&packets_[0], sample.size));
        }

    } // namespace mux
} // namespace ppbox
