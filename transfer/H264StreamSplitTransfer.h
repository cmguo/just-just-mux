// H264StreamSplitTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_H264_STREAM_SPLIT_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_H264_STREAM_SPLIT_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avcodec/avc/AvcNaluHelper.h>

namespace ppbox
{
    namespace mux
    {

        class H264StreamSplitTransfer
            : public Transfer
        {
        public:
            virtual void transfer(
                Sample & sample);

        private:
            ppbox::avcodec::AvcNaluHelper helper_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_H264_STREAM_SPLIT_TRANSFER_H_
