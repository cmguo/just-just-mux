// RtpEsMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpEsMuxer.h"
#include "ppbox/mux/transfer/PackageSplitTransfer.h"
#include "ppbox/mux/transfer/StreamSplitTransfer.h"
#include "ppbox/mux/rtp/RtpEsAudioTransfer.h"
#include "ppbox/mux/rtp/RtpAudioMpegTransfer.h"
#include "ppbox/mux/rtp/RtpEsVideoTransfer.h"
#include "ppbox/mux/transfer/PtsComputeTransfer.h"
#include "ppbox/mux/transfer/ParseH264Transfer.h"

using namespace ppbox::demux;

namespace ppbox
{
    namespace mux
    {

        RtpEsMuxer::RtpEsMuxer()
        {
        }

        RtpEsMuxer::~RtpEsMuxer()
        {
        }

        void RtpEsMuxer::add_stream(
            MediaInfoEx & mediainfo)
        {
            Transfer * transfer = NULL;
            if (mediainfo.type == ppbox::demux::MEDIA_TYPE_VIDE) {
                if (mediainfo.format_type == StreamInfo::video_avc_packet) {
                    transfer = new PackageSplitTransfer();
                    mediainfo.transfers.push_back(transfer);
                    //transfer = new ParseH264Transfer();
                    //mediainfo.transfers.push_back(transfer);
                } else if (mediainfo.format_type == StreamInfo::video_avc_byte_stream) {
                    transfer = new StreamSplitTransfer();
                    mediainfo.transfers.push_back(transfer);
                    transfer = new PtsComputeTransfer();
                    mediainfo.transfers.push_back(transfer);
                }
                RtpTransfer * rtp_transfer = new RtpEsVideoTransfer(*this);
                mediainfo.transfers.push_back(rtp_transfer);
                add_transfer(rtp_transfer);
            } else if (ppbox::demux::MEDIA_TYPE_AUDI == mediainfo.type){
                RtpTransfer * rtp_transfer = NULL;
                if (mediainfo.sub_type == demux::AUDIO_TYPE_MP1A) {
                    rtp_transfer = new RtpAudioMpegTransfer(*this);
                } else {
                    rtp_transfer = new RtpEsAudioTransfer(*this);
                }
                mediainfo.transfers.push_back(rtp_transfer);
                add_transfer(rtp_transfer);
            }
        }

    } // namespace mux
} // namespace ppbox
