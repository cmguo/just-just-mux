// Filter.h

#ifndef _JUST_MUX_FILTER_FILTER_H_
#define _JUST_MUX_FILTER_FILTER_H_

#include "just/mux/MuxBase.h"
#include "just/mux/MuxEvent.h"

#include <framework/container/List.h>

namespace just
{
    namespace mux
    {

        class Filter
            : public framework::container::ListHook<Filter>::type
        {
        public:
            Filter()
                : nref_(1)
            {
            }

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
                MuxEvent const & event, 
                boost::system::error_code & ec)
            {
                return next()->put(event, ec);
            }

        public:
            void attach()
            {
                ++nref_;
            }

            void detach()
            {
                if (--nref_ == 0) {
                    delete this;
                }
            }

        private:
            size_t nref_;
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_FILTER_FILTER_H_
