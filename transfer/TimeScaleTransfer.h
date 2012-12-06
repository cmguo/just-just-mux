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
            TimeScaleTransfer(
                boost::uint32_t time_scale);

            ~TimeScaleTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

            virtual void on_seek(
                boost::uint64_t time);

        protected:
            boost::uint8_t time_adjust_;
            boost::uint32_t sample_per_frame_;
            framework::system::ScaleTransform scale_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_TIME_SCALE_TRANSFER_H_