// FilterPipe.h

#ifndef _JUST_MUX_FILTER_FILTER_PIPE_H_
#define _JUST_MUX_FILTER_FILTER_PIPE_H_

#include "just/mux/Filter.h"

namespace just
{
    namespace mux
    {

        class FilterPipe
            : framework::container::List<Filter>
        {
        public:
            FilterPipe()
            {
            }

            ~FilterPipe()
            {
                while (!empty()) {
                    Filter * filter = last();
                    filter->unlink();
                    filter->detach();
                }
            }

        public:
            bool insert(
                Filter * filter, 
                bool adopt = true)
            {
                if (!adopt) {
                    filter->attach();
                }
                push_back(filter);
                return true;
            }

            bool remove(
                Filter * filter, 
                bool adopt)
            {
                erase(filter);
                filter->detach();
                return true;
            }

        public:
            void config(
                framework::configure::Config & conf)
            {
                first()->config(conf);
            }

        public:
            bool put(
                StreamInfo & info, 
                boost::system::error_code & ec)
            {
                return first()->put(info, ec);
            }

            bool put(
                Sample & sample, 
                boost::system::error_code & ec)
            {
                return first()->put(sample, ec);
            }

            bool put(
                MuxEvent const & event, 
                boost::system::error_code & ec)
            {
                return first()->put(event, ec);
            }
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_FILTER_FILTER_PIPE_H_
