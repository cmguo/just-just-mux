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
using namespace ppbox::avformat;

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
            StreamInfo & info)
        {
            Transfer * transfer = NULL;
            if (info.type == MEDIA_TYPE_VIDE) {
                if (info.format_type == StreamInfo::video_avc_packet) {
                    transfer = new PackageSplitTransfer();
                    info.transfers.push_back(transfer);
                    //transfer = new ParseH264Transfer();
                    //info.transfers.push_back(transfer);
                } else if (info.format_type == StreamInfo::video_avc_byte_stream) {
                    transfer = new StreamSplitTransfer();
                    info.transfers.push_back(transfer);
                    transfer = new PtsComputeTransfer();
                    info.transfers.push_back(transfer);
                }
                RtpTransfer * rtp_transfer = new RtpEsVideoTransfer(*this);
                info.transfers.push_back(rtp_transfer);
                add_transfer(rtp_transfer);
            } else if (MEDIA_TYPE_AUDI == info.type){
                RtpTransfer * rtp_transfer = NULL;
                if (info.sub_type == AUDIO_TYPE_MP1A) {
                    rtp_transfer = new RtpAudioMpegTransfer(*this);
                } else {
                    rtp_transfer = new RtpEsAudioTransfer(*this);
                }
                info.transfers.push_back(rtp_transfer);
                add_transfer(rtp_transfer);
            }
        }

    } // namespace mux
} // namespace ppbox
