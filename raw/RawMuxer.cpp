// RawMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/raw/RawMuxer.h"
#include "ppbox/mux/transfer/TimeScaleTransfer.h"

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
                << CONFIG_PARAM_NAME_RDWR("real_format", real_format_)
                << CONFIG_PARAM_NAME_RDWR("time_scale", time_scale_)
                << CONFIG_PARAM_NAME_RDWR("video_time_scale", video_time_scale_)
                << CONFIG_PARAM_NAME_RDWR("audio_time_scale", audio_time_scale_)
                ;
        }

        RawMuxer::~RawMuxer()
        {
        }

        void RawMuxer::do_open(
            MediaInfo & info)
        {
            real_format_.resize(4, '\0');
            format_type(*(boost::uint32_t const *)real_format_.c_str());
        }

        void RawMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            Transfer * transfer = NULL;
            if (info.type == StreamType::VIDE) {
                if (time_scale_ || video_time_scale_) {
                    transfer = new TimeScaleTransfer(time_scale_ ? time_scale_ : video_time_scale_);
                    transfers.push_back(transfer);
                }
            } else if (info.type == StreamType::AUDI) {
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
