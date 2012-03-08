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
            : time_scale_in_ms_(1)
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
            boost::uint32_t time, 
            boost::uint32_t play_time)
        {
            boost::uint32_t time_offset = time * time_scale_in_ms_;
            boost::uint32_t play_time_offset = (play_time) * time_scale_in_ms_;
            rtp_head_.timestamp += play_time_offset;
            rtp_info_.timestamp = rtp_head_.timestamp + time_offset;
            rtp_info_.sequence = rtp_head_.sequence;
            rtp_info_.seek_time = time;
        }

    }
}
