// M3U8Mux.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/raw/RawMuxer.h"
#include "ppbox/mux/transfer/PackageSplitTransfer.h"
#include "ppbox/mux/transfer/StreamSplitTransfer.h"
#include "ppbox/mux/transfer/PtsComputeTransfer.h"
#include "ppbox/mux/transfer/StreamJoinTransfer.h"
#include "ppbox/mux/transfer/AdtsAudioTransfer.h"

using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        RawMuxer::RawMuxer()
        {
            config().register_module("RawMuxer")
                << CONFIG_PARAM_NAME_RDWR("video_format", video_format_)
                << CONFIG_PARAM_NAME_RDWR("audio_format", audio_format_);
        }

        RawMuxer::~RawMuxer()
        {
        }

        void RawMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            Transfer * transfer = NULL;
            if (info.type == MEDIA_TYPE_VIDE) {
                if (info.format_type == StreamInfo::video_avc_packet) {
                    if (video_format_ == "es") {
                        transfer = new PackageSplitTransfer();
                        transfers.push_back(transfer);
                        transfer = new StreamJoinTransfer();
                        transfers.push_back(transfer);
                    }
                } else if (info.format_type == StreamInfo::video_avc_byte_stream) {
                    if (video_format_ == "es") {
                        transfer = new StreamSplitTransfer();
                        transfers.push_back(transfer);
                        transfer = new PtsComputeTransfer();
                        transfers.push_back(transfer);
                    }
                }
            } else if (info.type == MEDIA_TYPE_AUDI) {
                if (info.sub_type == AUDIO_TYPE_MP4A) {
                    if (audio_format_ == "adts") {
                        transfer = new AdtsAudioTransfer();
                        transfers.push_back(transfer);
                    }
                }
            }
        }

        void RawMuxer::file_header(
            Sample & sample)
        {
        }

        void RawMuxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
        }


    } // namespace mux
} // namespace ppbox
