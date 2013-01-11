// StreamSplitTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_STREAM_SPLIT_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_STREAM_SPLIT_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avformat/codec/avc/AvcNaluHelper.h>

namespace ppbox
{
    namespace mux
    {

        class StreamSplitTransfer
            : public Transfer
        {
        public:
            virtual void transfer(
                Sample & sample);

        private:
            ppbox::avformat::AvcNaluHelper helper_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_STREAM_SPLIT_TRANSFER_H_
