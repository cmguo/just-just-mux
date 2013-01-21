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
            virtual void config(
                framework::configure::Config & conf)
            {
                transfer_->config(conf);
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

            virtual void on_seek(
                boost::uint64_t time)
            {
                transfer_->on_seek(time);
            }

        private:
            Transfer * transfer_;
        };

    } // namespace mux
} // namespace ppbox

#endif // End _PPBOX_MUX_TRANSFER_MERGE_TRANSFER_H_
