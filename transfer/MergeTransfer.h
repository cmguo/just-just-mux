// MergeTransfer.h

#ifndef   _PPBOX_MUX_TRANSFER_MERGE_TRANSFER_H_
#define   _PPBOX_MUX_TRANSFER_MERGE_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"


namespace ppbox
{
    namespace mux
    {
        class MergeTransfer
            : public Transfer
        {
        public:
            MergeTransfer(
                Transfer * transfer)
                : transfer_(transfer)
            {
            }

            void transfer(ppbox::demux::Sample & sample)
            {
                transfer_->transfer(sample);
            }

        private:
            Transfer * transfer_;
        };

    }
}
#endif // End _PPBOX_MUX_TRANSFER_MERGE_TRANSFER_H_
