// Transfer.h

#ifndef _PPBOX_MUX_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_H_

#include "ppbox/mux/MuxBase.h"

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

        public:
            virtual void config(
                framework::configure::Config & conf)
            {
            }

        public:
            virtual void transfer(
                StreamInfo & info)
            {
            }

            virtual void transfer(
                Sample & sample) = 0;

            virtual void before_seek(
                Sample & sample)
            {
            }

            virtual void on_seek(
                boost::uint64_t time)
            {
            }
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_H_
