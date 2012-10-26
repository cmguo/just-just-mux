// RtpTsMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpTsMuxer.h"
#include "ppbox/mux/rtp/RtpTsTransfer.h"
#include "ppbox/mux/transfer/MergeTransfer.h"

#include <framework/string/Format.h>
using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        RtpTsMuxer::RtpTsMuxer()
            : RtpMuxer(&ts_mux_)
            , rtp_ts_transfer_(NULL)
        {
        }

        RtpTsMuxer::~RtpTsMuxer()
        {
            if (rtp_ts_transfer_) {
                delete rtp_ts_transfer_;
                rtp_ts_transfer_ = NULL;
            }
        }

        void RtpTsMuxer::add_stream(
            StreamInfo & info)
        {
            RtpMuxer::add_stream(info);
            if (rtp_ts_transfer_ == NULL) {
                rtp_ts_transfer_ = new RtpTsTransfer(*this);
                add_rtp_transfer(rtp_ts_transfer_);
            }
            Transfer * transfer = new MergeTransfer(rtp_ts_transfer_);
            add_transfer(info.index, *transfer);
        }

        void RtpTsMuxer::file_header(
            Sample & tag)
        {
            RtpMuxer::file_header(tag);
            rtp_ts_transfer_->header_rtp_packet(tag);
        }

    } // namespace mux
} // namespace ppbox
