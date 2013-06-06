// Transfer.h

#ifndef _PPBOX_MUX_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_H_

#include "ppbox/mux/Filter.h"

namespace ppbox
{
    namespace mux
    {

        class Transfer
            : public Filter
        {
        public:
            virtual void transfer(
                StreamInfo & info)
            {
            }

            virtual void transfer(
                Sample & sample) = 0;

            virtual void on_event(
                MuxEvent const & event)
            {
            }

        protected:
            virtual bool put(
                StreamInfo & info, 
                boost::system::error_code & ec)
            {
                transfer(info);
                return Filter::put(info, ec);
            }

            virtual bool put(
                Sample & sample, 
                boost::system::error_code & ec)
            {
                transfer(sample);
                return Filter::put(sample, ec);
            }

            virtual bool put(
                MuxEvent const & event, 
                boost::system::error_code & ec)
            {
                on_event(event);
                return Filter::put(event, ec);
            }
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_H_
