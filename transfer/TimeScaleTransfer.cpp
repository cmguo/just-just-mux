// TimeScaleTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/TimeScaleTransfer.h"

#include <ppbox/avformat/codec/aac/AacCodec.h>
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        TimeScaleTransfer::TimeScaleTransfer(
            boost::uint32_t time_scale)
            : time_adjust_mode_(1)
            , scale_out_(time_scale)
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
            if (info.index >= items_.size()) {
                items_.resize(info.index + 1);
            }
            Item & item = items_[info.index];
            if (info.type == MEDIA_TYPE_VIDE) {
                item.scale_.reset(info.time_scale, scale_out_);
            } else {
                if ((time_adjust_mode_ == 2) || 
                    (time_adjust_mode_ == 1 && info.time_scale < info.audio_format.sample_rate)) {
                        if (scale_out_ == 1) {
                            scale_out_ = info.audio_format.sample_rate;
                        }
                        item.scale_.reset(info.audio_format.sample_rate, scale_out_);
                        item.time_adjust_ = 1;
                         // TO DO:
                        item.sample_per_frame_ = 1024;
                        //if (info.sub_type == AUDIO_TYPE_MP4A) {
                        //    AacConfigHelper const & config = ((AacCodec const *)info.codec.get())->config_helper();
                        //    if (config.get_extension_object_type() == 5) {
                        //        item.sample_per_frame_ *= 2;
                        //    }
                        //}
                } else {
                    if (scale_out_ == 1) {
                        scale_out_ = info.time_scale;
                    }
                    item.scale_.reset(info.time_scale, scale_out_);
                }
            }
        }

        void TimeScaleTransfer::transfer(
            Sample & sample)
        {
            StreamInfo const & info = 
                *(StreamInfo const *)sample.stream_info;
            //std::cout << "sample track = " << sample.itrack << ", dts = " << sample.dts << ", cts_delta = " << sample.cts_delta << std::endl;
            Item & item = items_[info.index];
            if (item.time_adjust_ == 0) {
                sample.dts = item.scale_.transfer(sample.dts);
                boost::uint64_t cts = item.scale_.inc(sample.cts_delta);
                sample.cts_delta = (boost::uint32_t)(cts - sample.dts);
            } else if (item.time_adjust_ == 1 || (sample.flags & sample.discontinuity)) {
                sample.dts = item.scale_.static_transfer(info.time_scale, item.scale_.scale_out(), sample.dts);
                item.scale_.set(sample.dts);
                item.time_adjust_ = 2;
            } else {
                boost::uint64_t dts = item.scale_.static_transfer(info.time_scale, item.scale_.scale_out(), sample.dts);
                sample.dts = item.scale_.inc(item.sample_per_frame_);
                if (sample.dts > dts + item.scale_.scale_out() || sample.dts + item.scale_.scale_out() < dts) {
                    sample.dts = dts;
                    item.scale_.set(sample.dts);
                }
            }
            //std::cout << "sample track = " << sample.itrack << ", dts = " << sample.dts << ", cts_delta = " << sample.cts_delta << std::endl;
        }

        void TimeScaleTransfer::on_seek(
            boost::uint64_t time)
        {
            for (size_t i = 0; i < items_.size(); ++i) {
                Item & item = items_[i];
                if (item.time_adjust_ == 2)
                    item.time_adjust_ = 1;
            }
        }

        boost::uint32_t TimeScaleTransfer::scale_out()
        {
            return scale_out_;
        }

    } // namespace mux
} // namespace ppbox
