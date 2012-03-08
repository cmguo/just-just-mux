// RtpTsMux.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpTsMux.h"
#include "ppbox/mux/rtp/RtpTsTransfer.h"
#include "ppbox/mux/transfer/MergeTransfer.h"

#include <framework/string/Format.h>
using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        RtpTsMux::RtpTsMux()
            : RtpMux(&ts_mux_)
            , rtp_ts_transfer_(NULL)
        {
        }

        RtpTsMux::~RtpTsMux()
        {
            if (rtp_ts_transfer_) {
                delete rtp_ts_transfer_;
                rtp_ts_transfer_ = NULL;
            }
        }

        void RtpTsMux::add_stream(
            MediaInfoEx & mediainfo)
        {
            RtpMux::add_stream(mediainfo);
            if (rtp_ts_transfer_ == NULL) {
                rtp_ts_transfer_ = new RtpTsTransfer(*this);
                add_transfer(rtp_ts_transfer_);
            }
            Transfer * transfer = new MergeTransfer(rtp_ts_transfer_);
            mediainfo.transfers.push_back(transfer);
        }

        void RtpTsMux::file_header(
            ppbox::demux::Sample & tag)
        {
            RtpMux::file_header(tag);
            rtp_ts_transfer_->header_rtp_packet(tag);
        }

    } // namespace mux
} // namespace ppbox
