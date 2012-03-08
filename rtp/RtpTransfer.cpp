// RtpTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/rtp/RtpTransfer.h"

namespace ppbox
{
    namespace mux
    {

        RtpTransfer::RtpTransfer(
            Muxer & muxer, 
            std::string const & name, 
            boost::uint8_t type)
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

            muxer.config().register_module(name)
                << CONFIG_PARAM_NAME_RDWR("sequence", rtp_head_.sequence)
                << CONFIG_PARAM_NAME_RDWR("timestamp", rtp_head_.timestamp)
                << CONFIG_PARAM_NAME_RDWR("ssrc", rtp_head_.ssrc);
        }

        RtpTransfer::~RtpTransfer()
        {
        }

        void RtpTransfer::setup()
        {
            rtp_info_.setup = true;
        }

        void RtpTransfer::on_seek(
            boost::uint32_t time)
        {
            if (packets_.empty())
                return;
            boost::uint32_t last_timestamp = 
                framework::system::BytesOrder::host_to_big_endian(packets_[0].timestamp);
            std::cout << "last_timestamp = " << last_timestamp << std::endl;
            boost::uint64_t time2 = framework::system::ScaleTransform::static_transfer(1000, scale_.scale_in(), time);
            std::cout << "time2 = " << time2 << std::endl;
            rtp_info_.timestamp = last_timestamp + scale_.scale_out() * 8;
            std::cout << "rtp_info_.timestamp = " << rtp_info_.timestamp << std::endl;
            rtp_head_.timestamp = rtp_info_.timestamp - scale_.transfer(time2);
            std::cout << "rtp_head_.timestamp = " << rtp_head_.timestamp << std::endl;
            rtp_info_.sequence = rtp_head_.sequence;
            rtp_info_.seek_time = time;
        }

    } // namespace mux
} // namespace ppbox
