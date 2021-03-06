// TimeScaleTransfer.cpp

#include "just/mux/Common.h"
#include "just/mux/transfer/TimeScaleTransfer.h"

#include <just/avcodec/aac/AacCodec.h>
using namespace just::avcodec;

namespace just
{
    namespace mux
    {

        TimeScaleTransfer::TimeScaleTransfer(
            boost::uint32_t time_scale)
            : time_adjust_mode_(ta_auto)
            , scale_in_(0)
            , scale_out_(time_scale)
            , time_adjust_(0)
            , sample_per_frame_(0)
            , duration_in_(0)
            , duration_out_(0)
        {
        }

        TimeScaleTransfer::~TimeScaleTransfer()
        {
        }

        void TimeScaleTransfer::config(
            framework::configure::Config & conf)
        {
            conf.register_module("TimeScale")
                << CONFIG_PARAM_NAME_NOACC("time_adjust_mode", time_adjust_mode_);
        }

        void TimeScaleTransfer::transfer(
            StreamInfo & info)
        {
            scale_in_ = info.time_scale;
            if (scale_out_ == 0) {
                scale_out_ = scale_in_;
                scale_.reset_scale(scale_in_, scale_out_);
            } else if (info.type == StreamType::AUDI
                && info.audio_format.sample_per_frame > 0
                && ((time_adjust_mode_ == ta_enable) || 
                    (time_adjust_mode_ == ta_auto && scale_in_ < info.audio_format.sample_rate))) {
                        if (scale_out_ == 1) {
                            scale_out_ = info.audio_format.sample_rate;
                        }
                        scale_.reset_scale(info.audio_format.sample_rate, scale_out_);
                        time_adjust_ = 1;
                        sample_per_frame_ = info.audio_format.sample_per_frame;
            } else {
                if (scale_out_ == 1) {
                    scale_out_ = scale_in_;
                }
                scale_.reset_scale(scale_in_, scale_out_);
            }
            info.time_scale = scale_out_;
            if (info.type == StreamType::VIDE && info.video_format.frame_rate_num) {
                duration_in_ = (boost::uint32_t)(
                    (boost::uint64_t)scale_in_ * info.video_format.frame_rate_den / info.video_format.frame_rate_num);
                duration_out_ = (boost::uint32_t)(
                    (boost::uint64_t)scale_out_ * info.video_format.frame_rate_den / info.video_format.frame_rate_num);
            } else if (info.type == StreamType::AUDI && info.audio_format.sample_rate) {
                duration_in_ = (boost::uint32_t)(
                    (boost::uint64_t)scale_in_ * info.audio_format.sample_per_frame / info.audio_format.sample_rate);
                duration_out_ = (boost::uint32_t)(
                    (boost::uint64_t)scale_out_ * info.audio_format.sample_per_frame / info.audio_format.sample_rate);
            }
        }

        void TimeScaleTransfer::transfer(
            Sample & sample)
        {
            //std::cout << "sample track = " << sample.itrack << ", dts = " << sample.dts << ", cts_delta = " << sample.cts_delta << std::endl;
            if (time_adjust_ == 0) {
                sample.dts = scale_.transfer(sample.dts);
                boost::uint64_t cts = scale_.inc(sample.cts_delta);
                sample.cts_delta = (boost::uint32_t)(cts - sample.dts);
            } else if (time_adjust_ == 1 || (sample.flags & sample.f_discontinuity)) {
                sample.dts = scale_.static_transfer(scale_in_, scale_.scale_out(), sample.dts);
                scale_.last_out(sample.dts);
                time_adjust_ = 2;
            } else {
                //boost::uint64_t dts = scale_.static_transfer(scale_in_, scale_.scale_out(), sample.dts);
                sample.dts = scale_.inc(sample_per_frame_);
                //if (sample.dts > dts + scale_.scale_out() || sample.dts + scale_.scale_out() < dts) {
                //    sample.dts = dts;
                //    scale_.last_out(sample.dts);
                //}
            }
            if (sample.duration == duration_in_ || sample.duration == 0) {
                sample.duration = duration_out_;
            } else {
                sample.duration = (boost::uint32_t)(
                    (boost::uint64_t)scale_out_ * sample.duration / scale_in_);
            }
            //std::cout << "sample track = " << sample.itrack << ", dts = " << sample.dts << ", cts_delta = " << sample.cts_delta << std::endl;
        }

        void TimeScaleTransfer::on_event(
            MuxEvent const & event)
        {
            if (event.type == event.after_reset && time_adjust_ == 2)
                time_adjust_ = 1;
        }

    } // namespace mux
} // namespace just
