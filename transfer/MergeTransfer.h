// MergeTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_MERGE_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_MERGE_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

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

        public:
            virtual void transfer(
                StreamInfo & infoex)
            {
                transfer_->transfer(infoex);
            };

            void transfer(
                Sample & sample)
            {
                transfer_->transfer(sample);
            }

        private:
            Transfer * transfer_;
        };

    } // namespace mux
} // namespace ppbox

#endif // End _PPBOX_MUX_TRANSFER_MERGE_TRANSFER_H_
