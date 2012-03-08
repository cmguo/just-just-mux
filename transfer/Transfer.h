// Transfer.h

#ifndef _PPBOX_MUX_TRANSFER_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_TRANSFER_H_

#include "ppbox/mux/MuxerBase.h"

namespace ppbox
{
    namespace mux
    {

        class Muxer;

        class Transfer
        {
        public:
            Transfer()
            {
            }

            virtual ~Transfer()
            {
            }

            virtual void transfer(
                MediaInfoEx & mediainfo)
            {
            }

            virtual void transfer(
                ppbox::demux::Sample & sample) = 0;
        };

    }
}

#endif // _PPBOX_MUX_TRANSFER_TRANSFER_H_
