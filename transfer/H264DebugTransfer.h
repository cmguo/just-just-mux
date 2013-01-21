// H264DebugTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_H264_DEBUG_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_H264_DEBUG_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

namespace ppbox
{
    namespace mux
    {

        class H264DebugTransfer
            : public Transfer
        {
        public:
            virtual void transfer(
                StreamInfo & media);

            virtual void transfer(
                Sample & sample);
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_H264_DEBUG_TRANSFER_H_
