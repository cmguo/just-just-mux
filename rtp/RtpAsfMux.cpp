// RtpAsfMux.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpAsfMux.h"
#include "ppbox/mux/rtp/RtpAsfTransfer.h"
#include "ppbox/mux/transfer/MergeTransfer.h"

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        RtpAsfMux::RtpAsfMux()
            : RtpMux(&asf_mux_)
            , rtp_asf_transfer_(NULL)
        {
        }

        RtpAsfMux::~RtpAsfMux()
        {
            if (rtp_asf_transfer_) {
                delete rtp_asf_transfer_;
                rtp_asf_transfer_ = NULL;
            }
        }

        void RtpAsfMux::add_stream(
            MediaInfoEx & mediainfo)
        {
            RtpMux::add_stream(mediainfo);
            if (rtp_asf_transfer_ == NULL) {
                rtp_asf_transfer_ = new RtpAsfTransfer(*this);
                add_transfer(rtp_asf_transfer_);
            }
            Transfer * transfer = new MergeTransfer(rtp_asf_transfer_);
            mediainfo.transfers.push_back(transfer);
        }

        error_code RtpAsfMux::get_sdp(
            std::string & sdp_out, 
            error_code & ec)
        {
            ppbox::demux::Sample tag;
            RtpMux::read(tag, ec);
            if (!ec) {
                rtp_asf_transfer_->get_sdp(tag, sdp_out);
            }
            return ec;
        }

    } // namespace mux
} // namespace ppbox
