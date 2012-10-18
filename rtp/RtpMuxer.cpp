// RtpMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpMuxer.h"
#include "ppbox/mux/rtp/RtpTransfer.h"

#include <framework/string/Base16.h>
using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        RtpMuxer::RtpMuxer()
            : base_(NULL)
        {
        }

        RtpMuxer::RtpMuxer(
            MuxerBase * base)
            : base_(base)
        {
        }

        RtpMuxer::~RtpMuxer()
        {
        }

        void RtpMuxer::add_stream(
            StreamInfo & info)
        {
            if (base_) {
                ((RtpMuxer *)base_)->add_stream(info);
            }
        }

        void RtpMuxer::file_header(
            Sample & tag)
        {
            if (base_) {
                ((RtpMuxer *)base_)->file_header(tag);
            }
        }

        void RtpMuxer::stream_header(
            boost::uint32_t index, 
            Sample & tag)
        {
            if (base_) {
                ((RtpMuxer *)base_)->stream_header(index, tag);
            }
        }

        void RtpMuxer::add_transfer(
            RtpTransfer * rtp_transfer)
        {
            rtp_transfers_.push_back(rtp_transfer);
        }

        boost::system::error_code RtpMuxer::get_sdp(
            std::string & sdp_out, 
            boost::system::error_code & ec)
        {
            for(boost::uint32_t i = 0; i < rtp_transfers_.size(); ++i) {
                sdp_out += rtp_transfers_[i]->rtp_info().sdp;
            }
            ec.clear();
            return ec;
        }

        boost::system::error_code RtpMuxer::setup(
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

        boost::system::error_code RtpMuxer::get_rtp_info(
            std::string & rtp_info_out, 
            boost::uint32_t & seek_time, 
            boost::system::error_code & ec)
        {
            std::ostringstream os;
            for(boost::uint32_t i = 0; i < rtp_transfers_.size(); ++i) {
                RtpInfo const & rtp_info = rtp_transfers_[i]->rtp_info();
                if (rtp_info.setup) {
                    os << "url=" << rtp_info_out;
                    if(rtp_info_out[rtp_info_out.size()-1] == '/')
                        os << "track" << (boost::int32_t)rtp_info.stream_index;
                    else
                        os << "/track" << (boost::int32_t)rtp_info.stream_index;
                    os << ";seq=" << rtp_info.sequence;
                    os << ";rtptime=" << rtp_info.timestamp;
                    os << ",";
                    seek_time = rtp_info.seek_time;
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
