// PackageSplitTransfer.h

#ifndef _PPBOX_TRANSFER_MUX_PACKAGE_SPLIT_TRANSFER_H_
#define _PPBOX_TRANSFER_MUX_PACKAGE_SPLIT_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avformat/codec/avc/AvcNaluHelper.h>

namespace ppbox
{
    namespace mux
    {

        class PackageSplitTransfer
            : public Transfer
        {
        public:
            PackageSplitTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            ppbox::avformat::AvcNaluHelper helper_;
            std::vector<ppbox::avformat::NaluBuffer> nalus_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_TRANSFER_MUX_PACKAGE_SPLIT_TRANSFER_H_
