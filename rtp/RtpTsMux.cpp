// RtpTsMux.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpTsMux.h"
#include "ppbox/mux/transfer/Transfers.h"

#include <framework/string/Format.h>
using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        RtpTsMux::~RtpTsMux()
        {
            if (rtp_ts_transfer_) {
                delete rtp_ts_transfer_;
                rtp_ts_transfer_ = NULL;
            }
        }

        void RtpTsMux::add_stream(
            ppbox::demux::MediaInfo & mediainfo,
            std::vector<Transfer *> & transfers)
        {
            TsMux::add_stream(mediainfo, transfers);
            Transfer * transfer = NULL;
            transfer = new TsSplitTransfer();
            transfers.push_back(transfer);
            if (rtp_ts_transfer_ == NULL) {
                rtp_ts_transfer_ = new RtpTsTransfer(*this, map_id_);
            }
            transfer = new MergeTransfer(rtp_ts_transfer_);
            transfers.push_back(transfer);

            if (mediainfo.type == ppbox::demux::MEDIA_TYPE_VIDE) {
                mediainfo.attachment = NULL;
                rtp_ts_transfer_->get_rtp_info(mediainfo);
                Muxer::mediainfo().attachment = mediainfo.attachment;
            } else {
                mediainfo.attachment = NULL;
            }
        }

        void RtpTsMux::head_buffer(ppbox::demux::Sample & tag)
        {
            TsMux::head_buffer(tag);
            rtp_ts_transfer_->header_rtp_packet(tag);
        }

        error_code RtpTsMux::seek(
            boost::uint32_t & time,
            error_code & ec)
        {
            boost::uint32_t play_time = Muxer::current_time();
            Muxer::seek(time, ec);
            if (!ec || ec == boost::asio::error::would_block) {
                rtp_ts_transfer_->on_seek(time, play_time);
            }
            return ec;
        }


    } // namespace mux
} // namespace ppbox
