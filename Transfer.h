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

            virtual void reset(
                boost::uint64_t time)
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

            virtual bool reset(
                Sample & sample, 
                boost::system::error_code & ec)
            {
                reset(sample.time);
                return Filter::reset(sample, ec);
            }
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_H_
