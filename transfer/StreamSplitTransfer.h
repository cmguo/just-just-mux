// StreamSplitTransfer.h

#ifndef   _PPBOX_MUX_STREAM_SPLIT_TRANSFER_H_
#define   _PPBOX_MUX_STREAM_SPLIT_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/detail/BitsReader.h"

namespace ppbox
{
    namespace mux
    {

        class StreamSplitTransfer
            : public Transfer
        {
        public:
            StreamSplitTransfer()
            {
            }

            ~StreamSplitTransfer()
            {
            }

            virtual void transfer(ppbox::demux::Sample & sample);

        private:
            NaluList nalus_;
        };
    }
}
#endif
