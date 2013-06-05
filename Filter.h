// Filter.h

#ifndef _PPBOX_MUX_FILTER_FILTER_H_
#define _PPBOX_MUX_FILTER_FILTER_H_

#include "ppbox/mux/MuxBase.h"

#include <framework/container/List.h>

namespace ppbox
{
    namespace mux
    {

        class Filter
            : public framework::container::ListHook<Filter>::type
        {
        public:
            struct eos_t {};

            static eos_t eos()
            {
                return eos_t();
            }

        public:
            virtual ~Filter()
            {
            }

        public:
            virtual void config(
                framework::configure::Config & conf)
            {
                next()->config(conf);
            }

        public:
            virtual bool put(
                StreamInfo & info, 
                boost::system::error_code & ec)
            {
                return next()->put(info, ec);
            }

            virtual bool put(
                Sample & sample, 
                boost::system::error_code & ec)
            {
                return next()->put(sample, ec);
            }

            virtual bool put(
                eos_t & eos, 
                boost::system::error_code & ec)
            {
                return next()->put(eos, ec);
            }

            virtual bool reset(
                Sample & sample, 
                boost::system::error_code & ec)
            {
                return next()->reset(sample, ec);
            }

        protected:
            void detach_self()
            {
                unlink();
            }
        };

        typedef framework::container::List<Filter> FilterPipe;

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FILTER_FILTER_H_
