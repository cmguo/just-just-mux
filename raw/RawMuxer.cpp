// RawMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/raw/RawMuxer.h"
#include "ppbox/mux/raw/RawFormat.h"

namespace ppbox
{
    namespace mux
    {

        RawMuxer::RawMuxer(
            boost::asio::io_service & io_svc)
            : Muxer(io_svc)
        {
            format_ = new RawFormat;
            config().register_module("RawMuxer")
                << CONFIG_PARAM_NAME_RDWR("real_format", format_->real_format_)
                << CONFIG_PARAM_NAME_RDWR("time_scale", format_->time_scale_)
                << CONFIG_PARAM_NAME_RDWR("video_time_scale", format_->video_time_scale_)
                << CONFIG_PARAM_NAME_RDWR("audio_time_scale", format_->audio_time_scale_)
                ;
        }

        RawMuxer::~RawMuxer()
        {
        }

        void RawMuxer::do_open(
            MediaInfo & info)
        {
            format_->open();
            format(format_);
        }

        void RawMuxer::add_stream(
            StreamInfo & info, 
            FilterPipe & pipe)
        {
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
