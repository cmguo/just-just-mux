// Transfer.h

#ifndef _PPBOX_MUX_TRANSFER_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_TRANSFER_H_

#include "ppbox/mux/MuxBase.h"

namespace ppbox
{
    namespace mux
    {

        class MuxerBase;

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

            virtual void on_seek(
                boost::uint32_t time)
            {
            }
        };

    }
}

#endif // _PPBOX_MUX_TRANSFER_TRANSFER_H_
