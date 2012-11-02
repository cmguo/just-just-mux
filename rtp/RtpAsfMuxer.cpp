// RtpAsfMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpAsfMuxer.h"
#include "ppbox/mux/rtp/RtpAsfTransfer.h"
#include "ppbox/mux/transfer/MergeTransfer.h"

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        RtpAsfMuxer::RtpAsfMuxer()
            : RtpMuxer(&asf_mux_)
            , rtp_asf_transfer_(NULL)
        {
        }

        RtpAsfMuxer::~RtpAsfMuxer()
        {
            if (rtp_asf_transfer_) {
                delete rtp_asf_transfer_;
                rtp_asf_transfer_ = NULL;
            }
        }

        void RtpAsfMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            RtpMuxer::add_stream(info, transfers);
            if (rtp_asf_transfer_ == NULL) {
                rtp_asf_transfer_ = new RtpAsfTransfer(*this);
                add_rtp_transfer(rtp_asf_transfer_);
            }
            Transfer * transfer = new MergeTransfer(rtp_asf_transfer_);
            transfers.push_back(transfer);
        }

        error_code RtpAsfMuxer::get_sdp(
            std::string & sdp_out, 
            error_code & ec)
        {
            Sample tag;
            RtpMuxer::read(tag, ec);
            if (!ec) {
                rtp_asf_transfer_->get_sdp(tag, sdp_out);
            }
            return ec;
        }

    } // namespace mux
} // namespace ppbox
