// LastFilter.h

#ifndef _JUST_MUX_FILTER_LAST_FILTER_H_
#define _JUST_MUX_FILTER_LAST_FILTER_H_

#include "just/mux/Filter.h"

namespace just
{
    namespace mux
    {

        class LastFilter
            : public Filter
        {
        public:
            LastFilter(
                FilterManager & manager)
                : manager_(manager)
            {
            }

        public:
            virtual void config(
                framework::configure::Config & conf)
            {
            }

        public:
            virtual bool put(
                StreamInfo & info, 
                boost::system::error_code & ec)
            {
                ec.clear();
                return true;
            }

            virtual bool put(
                Sample & sample, 
                boost::system::error_code & ec)
            {
                return manager_.put(sample, ec);
            }

            virtual bool put(
                MuxEvent const & event, 
                boost::system::error_code & ec)
            {
                return manager_.put(event, ec);
            }

        private:
            FilterManager & manager_;
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_FILTER_LAST_FILTER_H_
