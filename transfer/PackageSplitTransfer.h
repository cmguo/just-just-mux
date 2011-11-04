// PackageSplitTransfer.h

#ifndef   _PPBOX_MUX_PACKAGE_SPLIT_TRANSFER_H_
#define   _PPBOX_MUX_PACKAGE_SPLIT_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/detail/BitsReader.h"

namespace ppbox
{
    namespace mux
    {

        class PackageSplitTransfer
            : public Transfer
        {
        public:
            PackageSplitTransfer()
                : nalu_length_(4)
            {
            }

            ~PackageSplitTransfer()
            {
            }

            virtual void transfer(ppbox::demux::Sample & sample);

        private:
            boost::uint32_t nalu_length_;
            NaluList nalus_;
        };
    }
}
#endif
