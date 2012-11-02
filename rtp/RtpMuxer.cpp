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

        bool RtpMuxer::setup(
            boost::uint32_t index, 
            boost::system::error_code & ec)
        {
            ec = framework::system::logic_error::out_of_range;
            for(size_t i = 0; i < rtp_transfers_.size(); ++i) {
                RtpInfo const & rtp_info = rtp_transfers_[i]->rtp_info();
                if (rtp_info.stream_index == index) {
                    rtp_transfers_[i]->setup();
                    ec.clear();
                }
            }
            return !ec;
        }

        void RtpMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            if (base_) {
                ((RtpMuxer *)base_)->add_stream(info, transfers); // 强制转换为RtpMuxer，是为了访问MuxerBase的protected成员
            }
        }

        void RtpMuxer::file_header(
            Sample & sample)
        {
            if (base_) {
                ((RtpMuxer *)base_)->file_header(sample);
            }
        }

        void RtpMuxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
            if (base_) {
                ((RtpMuxer *)base_)->stream_header(index, sample);
            }
        }

        void RtpMuxer::add_rtp_transfer(
            RtpTransfer * rtp_transfer)
        {
            rtp_transfers_.push_back(rtp_transfer);
        }

        void RtpMuxer::media_info(
            MediaInfo & info) const
        {
            MuxerBase::media_info(info);
            for(boost::uint32_t i = 0; i < rtp_transfers_.size(); ++i) {
                info.format_data += rtp_transfers_[i]->rtp_info().sdp;
            }
        }

        void RtpMuxer::play_info(
            PlayInfo & info) const
        {
            std::string config = info.config;
            MuxerBase::play_info(info);
            if (config.compare(0, 5, "ssrc=") == 0) {
                boost::uint32_t index = framework::string::parse<boost::uint32_t>(config.substr(5));
                if (index < rtp_transfers_.size()) {
                    RtpInfo const & rtp_info = rtp_transfers_[index]->rtp_info();
                    info.config = "ssrc=" 
                        + framework::string::Base16::encode(std::string((char const *)&rtp_info.ssrc, 4));
                }
            } else {
                std::ostringstream os;
                if (config[config.size()-1] != '/') {
                    config.append("/");
                }
                for(boost::uint32_t i = 0; i < rtp_transfers_.size(); ++i) {
                    RtpInfo const & rtp_info = rtp_transfers_[i]->rtp_info();
                    if (rtp_info.setup) {
                        os << "url=" << config;
                        os << "track" << (boost::int32_t)rtp_info.stream_index;
                        os << ";seq=" << rtp_info.sequence;
                        os << ";rtptime=" << rtp_info.timestamp;
                        os << ",";
                    }
                }
                info.config = os.str();
                if (!info.config.empty()) {
                    info.config.erase(--info.config.end(), info.config.end());
                }
            }
        }
 
    } // namespace mux
} // namespace ppbox
