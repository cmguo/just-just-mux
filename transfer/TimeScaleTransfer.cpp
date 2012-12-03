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
            : time_adjust_(0)
            , scale_(1, time_scale)
        {
        }

        TimeScaleTransfer::~TimeScaleTransfer()
        {
        }

        void TimeScaleTransfer::transfer(
            StreamInfo & info)
        {
            if (info.type == MEDIA_TYPE_VIDE) {
                scale_.reset(info.time_scale, scale_.scale_out());
            } else {
                if (info.time_scale < info.audio_format.sample_rate) {
                    scale_.reset(info.audio_format.sample_rate, scale_.scale_out());
                    time_adjust_ = 1;
                     // TO DO:
                    sample_per_frame_ = 1024;
                } else {
                    scale_.reset(info.time_scale, scale_.scale_out());
                }
            }
        }

        void TimeScaleTransfer::transfer(
            Sample & sample)
        {
            StreamInfo const & media = 
                *(StreamInfo const *)sample.stream_info;
            //std::cout << "sample track = " << sample.itrack << ", dts = " << sample.dts << ", cts_delta = " << sample.cts_delta << std::endl;
            if (time_adjust_ == 0) {
                sample.dts = scale_.transfer(sample.dts);
                boost::uint64_t cts = scale_.inc(sample.cts_delta);
                sample.cts_delta = (boost::uint32_t)(cts - sample.dts);
            } else if (time_adjust_ == 1) {
                sample.dts = scale_.static_transfer(media.time_scale, scale_.scale_out(), sample.dts);
                scale_.set(sample.dts);
                time_adjust_ = 2;
            } else {
                sample.dts = scale_.inc(sample_per_frame_);
            }
            //std::cout << "sample track = " << sample.itrack << ", dts = " << sample.dts << ", cts_delta = " << sample.cts_delta << std::endl;
        }

        void TimeScaleTransfer::on_seek(
            boost::uint32_t time)
        {
            if (time_adjust_ == 2)
                time_adjust_ = 1;
        }

    } // namespace mux
} // namespace ppbox
