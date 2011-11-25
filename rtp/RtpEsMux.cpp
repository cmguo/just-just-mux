// RtpEsMux.cpp
#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpEsMux.h"
#include "ppbox/mux/transfer/PackageSplitTransfer.h"
#include "ppbox/mux/transfer/PackageJoinTransfer.h"
#include "ppbox/mux/transfer/StreamSplitTransfer.h"
#include "ppbox/mux/rtp/RtpEsAudioTransfer.h"
#include "ppbox/mux/rtp/RtpEsVideoTransfer.h"
#include "ppbox/mux/transfer/PtsComputeTransfer.h"

#include <ppbox/demux/pptv/PptvDemuxer.h>
using namespace ppbox::demux;

namespace ppbox
{
    namespace mux
    {

        void RtpEsMux::add_stream(
            MediaInfo & mediainfo,
            std::vector<Transfer *> & transfers)
        {
            Transfer * transfer = NULL;
            if (mediainfo.type == ppbox::demux::MEDIA_TYPE_VIDE) {
                if (mediainfo.format_type == MediaInfo::video_avc_packet) {
                    transfer = new PackageSplitTransfer();
                    transfers.push_back(transfer);
                } else if (mediainfo.format_type == MediaInfo::video_avc_byte_stream) {
                    transfer = new StreamSplitTransfer();
                    transfers.push_back(transfer);

                    transfer = new PtsComputeTransfer();
                    transfers.push_back(transfer);

                    //transfer = new PackageJoinTransfer();
                    //transfers.push_back(transfer);
                    //transfer = new PackageSplitTransfer();
                    //transfers.push_back(transfer);
                }
                RtpTransfer * rtp_transfer = new RtpEsVideoTransfer(*this, video_map_id_);
                transfers.push_back(rtp_transfer);
                rtp_transfer->get_rtp_info(mediainfo);
                transfers_.push_back(rtp_transfer);
            } else {
                RtpTransfer * rtp_transfer = new RtpEsAudioTransfer(*this, audio_map_id_);
                transfers.push_back(rtp_transfer);
                rtp_transfer->get_rtp_info(mediainfo);
                transfers_.push_back(rtp_transfer);
            }
        }

        void RtpEsMux::head_buffer(ppbox::demux::Sample & tag)
        {
            tag.data.clear();
        }

        error_code RtpEsMux::seek(
            boost::uint32_t & time,
            error_code & ec)
        {
            boost::uint32_t play_time = Muxer::current_time();
            Muxer::seek(time, ec);
            if (!ec || ec == boost::asio::error::would_block) {
                for(boost::uint32_t i = 0; i < transfers_.size(); ++i) {
                    transfers_[i]->on_seek(time, play_time);
                }
            }
            return ec;
        }

    } // namespace mux
} // namespace ppbox
