// RtpTsMux.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpTsMux.h"
#include "ppbox/mux/transfer/MergeTransfer.h"
#include "ppbox/mux/rtp/RtpTsTransfer.h"

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
            MediaInfoEx & mediainfo)
        {
            TsMux::add_stream(mediainfo);
            if (rtp_ts_transfer_ == NULL) {
                rtp_ts_transfer_ = new RtpTsTransfer(*this, map_id_);
            }
            Transfer * transfer = new MergeTransfer(rtp_ts_transfer_);
            mediainfo.transfers.push_back(transfer);

            if (mediainfo.type == ppbox::demux::MEDIA_TYPE_VIDE) {
                rtp_ts_transfer_->get_rtp_info(mediainfo);
                Muxer::mediainfo().attachment = mediainfo.attachment;
            } else {
                mediainfo.attachment = NULL;
            }
        }

        void RtpTsMux::file_header(ppbox::demux::Sample & tag)
        {
            TsMux::file_header(tag);
            rtp_ts_transfer_->header_rtp_packet(tag);
        }

        void RtpTsMux::stream_header(
            boost::uint32_t index, 
            ppbox::demux::Sample & tag)
        {
            tag.data.clear();
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
