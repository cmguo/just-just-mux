// H264PackageSplitTransfer.h

#ifndef _PPBOX_TRANSFER_MUX_H264_PACKAGE_SPLIT_TRANSFER_H_
#define _PPBOX_TRANSFER_MUX_H264_PACKAGE_SPLIT_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avcodec/avc/AvcNaluHelper.h>

namespace ppbox
{
    namespace mux
    {

        class H264PackageSplitTransfer
            : public Transfer
        {
        public:
            H264PackageSplitTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            ppbox::avcodec::AvcNaluHelper helper_;
            std::vector<ppbox::avcodec::NaluBuffer> nalus_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_TRANSFER_MUX_H264_PACKAGE_SPLIT_TRANSFER_H_
