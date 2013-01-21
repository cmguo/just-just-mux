// RtpRawMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtp/RtpRawMuxer.h"
#include "ppbox/mux/transfer/H264PackageSplitTransfer.h"
#include "ppbox/mux/transfer/H264StreamSplitTransfer.h"
#include "ppbox/mux/transfer/H264PtsComputeTransfer.h"
#include "ppbox/mux/transfer/H264DebugTransfer.h"
#include "ppbox/mux/transfer/MpegAudioAdtsDecodeTransfer.h"
#include "ppbox/mux/rtp/RtpMpeg4GenericTransfer.h"
#include "ppbox/mux/rtp/RtpMpegAudioTransfer.h"
#include "ppbox/mux/rtp/RtpH264Transfer.h"

using namespace ppbox::demux;
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        RtpRawMuxer::RtpRawMuxer()
        {
        }

        RtpRawMuxer::~RtpRawMuxer()
        {
        }

        void RtpRawMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            Transfer * transfer = NULL;
            if (info.type == MEDIA_TYPE_VIDE) {
                if (info.format_type == StreamInfo::video_avc_packet) {
                    transfer = new H264PackageSplitTransfer();
                    transfers.push_back(transfer);
                    //transfer = new ParseH264Transfer();
                    //transfers.push_back(transfer);
                } else if (info.format_type == StreamInfo::video_avc_byte_stream) {
                    transfer = new H264StreamSplitTransfer();
                    transfers.push_back(transfer);
                    transfer = new H264PtsComputeTransfer();
                    transfers.push_back(transfer);
                }
                RtpTransfer * rtp_transfer = new RtpH264Transfer;
                transfers.push_back(rtp_transfer);
                add_rtp_transfer(rtp_transfer);
            } else if (MEDIA_TYPE_AUDI == info.type){
                RtpTransfer * rtp_transfer = NULL;
                if (info.sub_type == AUDIO_TYPE_MP1A) {
                    rtp_transfer = new RtpMpegAudioTransfer;
                } else {
                    if (info.format_type == StreamInfo::audio_aac_adts) {
                        transfer = new MpegAudioAdtsDecodeTransfer();
                        transfers.push_back(transfer);
                    }
                    rtp_transfer = new RtpMpeg4GenericTransfer;
                }
                transfers.push_back(rtp_transfer);
                add_rtp_transfer(rtp_transfer);
            } else {
                add_rtp_transfer(NULL);
            }
        }

    } // namespace mux
} // namespace ppbox
