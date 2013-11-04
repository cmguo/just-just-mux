// TimeScaleTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_TIME_SCALE_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_TIME_SCALE_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <framework/system/ScaleTransform.h>

namespace ppbox
{
    namespace mux
    {

        class TimeScaleTransfer
            : public Transfer
        {
        public:
            // scale_out = 0, meanings use scale in for scale out
            // scale_out = 1, meanings use scale adjust for scale out
            // if audio and time adjust mode enable, scale adjust is set to sample rate, 
            // else scale adjust is scale in
            TimeScaleTransfer(
                boost::uint32_t scale_out = 0);

            ~TimeScaleTransfer();

        public:
            virtual void config(
                framework::configure::Config & conf);

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

            virtual void on_event(
                MuxEvent const & event);

        protected:
            enum TimeAdjustModeEnum
            {
                ta_auto, 
                ta_disable, 
                ta_enable, 
            };
            boost::uint32_t time_adjust_mode_;
            boost::uint32_t scale_in_;
            boost::uint32_t scale_out_;
            boost::uint32_t time_adjust_;
            boost::uint32_t sample_per_frame_;
            framework::system::ScaleTransform scale_;
            boost::uint32_t duration_in_;
            boost::uint32_t duration_out_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_TIME_SCALE_TRANSFER_H_
