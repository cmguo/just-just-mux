// PackageSplitTransfer.h

#ifndef _PPBOX_TRANSFER_MUX_PACKAGE_SPLIT_TRANSFER_H_
#define _PPBOX_TRANSFER_MUX_PACKAGE_SPLIT_TRANSFER_H_

#include "ppbox/mux/Transfer.h"
#include "ppbox/mux/detail/BitsReader.h"

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
                Sample & sample);

        private:
            boost::uint32_t nalu_length_;
            NaluList nalus_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_TRANSFER_MUX_PACKAGE_SPLIT_TRANSFER_H_
