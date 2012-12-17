// TimeScaleTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/TimeScaleTransfer.h"

using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        TimeScaleTransfer::TimeScaleTransfer(
            boost::uint32_t time_scale)
            : scale_out_(time_scale)
        {
        }

        TimeScaleTransfer::~TimeScaleTransfer()
        {
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
                if (info.time_scale < info.audio_format.sample_rate) {
                    if (scale_out_ == 1) {
                        scale_out_ = info.audio_format.sample_rate;
                    }
                    item.scale_.reset(info.audio_format.sample_rate, scale_out_);
                    item.time_adjust_ = 1;
                     // TO DO:
                    item.sample_per_frame_ = 1024;
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
            } else if (item.time_adjust_ == 1) {
                sample.dts = item.scale_.static_transfer(info.time_scale, item.scale_.scale_out(), sample.dts);
                item.scale_.set(sample.dts);
                item.time_adjust_ = 2;
            } else {
                sample.dts = item.scale_.inc(item.sample_per_frame_);
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
