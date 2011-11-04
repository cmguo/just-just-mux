// Transfer.h

#ifndef   _PPBOX_MUX_TRANSFER_TRANSFER_H_
#define   _PPBOX_MUX_TRANSFER_TRANSFER_H_

#include "ppbox/mux/MuxerBase.h"

#include <util/buffers/BufferCopy.h>
#include <util/buffers/BufferSize.h>

namespace ppbox
{
    namespace mux
    {
        class Transfer
        {
        public:
            Transfer()
            {
            }

            virtual ~Transfer()
            {
            }

            virtual void transfer(ppbox::demux::Sample & sample) = 0;

        };
    }
}

#endif
