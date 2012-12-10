// RtpMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpMuxer.h"
#include "ppbox/mux/rtp/RtpTransfer.h"
#include "ppbox/mux/rtp/RtpStreamDesc.h"

#include <framework/string/Base16.h>
#include <framework/string/Format.h>
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

        void RtpMuxer::stream_info(
            std::vector<StreamInfo> & streams) const
        {
            MuxerBase::stream_info(streams);
            for(boost::uint32_t i = 0; i < rtp_transfers_.size(); ++i) {
                RtpInfo const & rtp_info = rtp_transfers_[i]->rtp_info();
                RtpStreamDesc desc;
                desc.stream = "track" + framework::string::format(rtp_info.stream_index);
                desc.ssrc = rtp_info.ssrc;
                desc.setup = rtp_info.setup;
                desc.sdp_info = rtp_info.sdp;
                std::ostringstream os;
                os << "seq=" << rtp_info.sequence;
                os << ";rtptime=" << rtp_info.timestamp;
                desc.rtp_info = os.str();
                desc.to_data(streams[i].format_data);
            }
        }

    } // namespace mux
} // namespace ppbox
