// StreamSplitTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/StreamSplitTransfer.h"

namespace ppbox
{
    namespace mux
    {

        void StreamSplitTransfer::transfer(
            Sample & sample)
        {
            helper_.from_stream(sample.size, sample.data);
            sample.context = (void*)&helper_.nalus();
        }

    } // namespace mux
} // namespace ppbox
