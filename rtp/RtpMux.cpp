// RtpMux.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpMux.h"
#include "ppbox/mux/rtp/RtpTransfer.h"

#include <framework/string/Base16.h>
using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        RtpMux::RtpMux()
            : base_(NULL)
        {
        }

        RtpMux::RtpMux(
            Muxer * base)
            : base_(base)
        {
        }

        RtpMux::~RtpMux()
        {
        }

        void RtpMux::add_stream(
            MediaInfoEx & mediainfo)
        {
            if (base_) {
                ((RtpMux *)base_)->add_stream(mediainfo);
            }
        }

        void RtpMux::file_header(
            ppbox::demux::Sample & tag)
        {
            if (base_) {
                ((RtpMux *)base_)->file_header(tag);
            }
        }

        void RtpMux::stream_header(
            boost::uint32_t index, 
            ppbox::demux::Sample & tag)
        {
            if (base_) {
                ((RtpMux *)base_)->stream_header(index, tag);
            }
        }

        error_code RtpMux::seek(
            boost::uint32_t & time,
            error_code & ec)
        {
            boost::uint32_t play_time = Muxer::current_time();
            Muxer::seek(time, ec);
            if (!ec || ec == boost::asio::error::would_block) {
                for(boost::uint32_t i = 0; i < rtp_transfers_.size(); ++i) {
                    rtp_transfers_[i]->on_seek(time, play_time);
                }
            }
            return ec;
        }

        void RtpMux::add_transfer(
            RtpTransfer * rtp_transfer)
        {
            rtp_transfers_.push_back(rtp_transfer);
        }

        boost::system::error_code RtpMux::get_sdp(
            std::string & sdp_out, 
            boost::system::error_code & ec)
        {
            for(boost::uint32_t i = 0; i < rtp_transfers_.size(); ++i) {
                sdp_out += rtp_transfers_[i]->rtp_info().sdp;
            }
            ec.clear();
            return ec;
        }

        boost::system::error_code RtpMux::setup(
            boost::uint32_t index, 
            std::string & setup_out, 
            boost::system::error_code & ec)
        {
            for(boost::uint32_t i = 0; i < rtp_transfers_.size(); ++i) {
                RtpInfo const & rtp_info = rtp_transfers_[i]->rtp_info();
                if (rtp_info.stream_index == index) {
                    rtp_transfers_[i]->setup();
                    setup_out = "ssrc=" 
                        + framework::string::Base16::encode(std::string((char const *)&rtp_info.ssrc, 4));
                    ec.clear();
                }
            }
            return ec;
        }

        boost::system::error_code RtpMux::get_rtp_info(
            std::string & rtp_info_out, 
            boost::system::error_code & ec)
        {
            std::ostringstream os;
            for(boost::uint32_t i = 0; i < rtp_transfers_.size(); ++i) {
                RtpInfo const & rtp_info = rtp_transfers_[i]->rtp_info();
                if (rtp_info.setup) {
                    os << "url=" << rtp_info_out;
                    os << "/index=" << rtp_info.stream_index;
                    os << ";seq=" << rtp_info.sequence;
                    os << ";rtptime=" << rtp_info.timestamp;
                    os << ",";
                }
            }
            rtp_info_out = os.str();
            if (!rtp_info_out.empty()) {
                rtp_info_out.erase(--rtp_info_out.end(), rtp_info_out.end());
            }
            ec.clear();
            return ec;
        }
 
    } // namespace mux
} // namespace ppbox
