// RawMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/raw/RawMuxer.h"
#include "ppbox/mux/transfer/H264PackageSplitTransfer.h"
#include "ppbox/mux/transfer/H264PackageJoinTransfer.h"
#include "ppbox/mux/transfer/H264StreamSplitTransfer.h"
#include "ppbox/mux/transfer/H264PtsComputeTransfer.h"
#include "ppbox/mux/transfer/H264StreamJoinTransfer.h"
#include "ppbox/mux/transfer/MpegAudioAdtsEncodeTransfer.h"
#include "ppbox/mux/transfer/MpegAudioAdtsDecodeTransfer.h"
#include "ppbox/mux/transfer/TimeScaleTransfer.h"
#include "ppbox/mux/filter/AdtsSplitFilter.h"

#include <ppbox/avformat/Format.h>
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        RawMuxer::RawMuxer()
            : time_scale_(0)
            , video_time_scale_(0)
            , audio_time_scale_(0)
        {
            config().register_module("RawMuxer")
                << CONFIG_PARAM_NAME_RDWR("video_format", video_format_)
                << CONFIG_PARAM_NAME_RDWR("audio_format", audio_format_)
                << CONFIG_PARAM_NAME_RDWR("time_scale", time_scale_)
                << CONFIG_PARAM_NAME_RDWR("video_time_scale", video_time_scale_)
                << CONFIG_PARAM_NAME_RDWR("audio_time_scale", audio_time_scale_)
                ;
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
                if (info.sub_type == VIDEO_TYPE_AVC1) {
                    if (info.format_type == FormatType::video_avc_packet) {
                        if (video_format_ == "es") {
                            transfer = new H264PackageSplitTransfer();
                            transfers.push_back(transfer);
                            transfer = new H264StreamJoinTransfer();
                            transfers.push_back(transfer);
                        }
                    } else if (info.format_type == FormatType::video_avc_byte_stream) {
                        if (video_format_ == "es") {
                            transfer = new H264StreamSplitTransfer();
                            transfers.push_back(transfer);
                            transfer = new H264PtsComputeTransfer();
                            transfers.push_back(transfer);
                        } else {
                            transfer = new H264StreamSplitTransfer();
                            transfers.push_back(transfer);
                            transfer = new H264PtsComputeTransfer();
                            transfers.push_back(transfer);
                            transfer = new H264PackageJoinTransfer();
                            transfers.push_back(transfer);
                        }
                    }
                }
                if (time_scale_ || video_time_scale_) {
                    transfer = new TimeScaleTransfer(time_scale_ ? time_scale_ : video_time_scale_);
                    transfers.push_back(transfer);
                }
            } else if (info.type == MEDIA_TYPE_AUDI) {
                if (info.sub_type == AUDIO_TYPE_MP4A) {
                    if (info.format_type == FormatType::audio_aac_adts) {
                        if (audio_format_ != "adts") {
                            add_filter(new AdtsSplitFilter);
                            transfer = new MpegAudioAdtsDecodeTransfer();
                            transfers.push_back(transfer);
                        }
                    }
                    if (audio_format_ == "adts") {
                        if (info.format_type != FormatType::audio_aac_adts) {
                            transfer = new MpegAudioAdtsEncodeTransfer();
                            transfers.push_back(transfer);
                        }
                    }
                }
                if (time_scale_ || audio_time_scale_) {
                    transfer = new TimeScaleTransfer(time_scale_ ? time_scale_ : audio_time_scale_);
                    transfers.push_back(transfer);
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
