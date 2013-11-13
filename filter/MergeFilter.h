// MergeFilter.h

#ifndef _PPBOX_MUX_FILTER_MERGE_FILTER_H_
#define _PPBOX_MUX_FILTER_MERGE_FILTER_H_

#include "ppbox/mux/Filter.h"

namespace ppbox
{
    namespace mux
    {

        class MergeHook;

        class MergeFilter
            : public Filter
        {
        public:
            MergeFilter(
                Filter * filter);

            virtual ~MergeFilter();

        public:
            virtual void config(
                framework::configure::Config & conf);

        public:
            virtual bool put(
                StreamInfo & info, 
                boost::system::error_code & ec);

            virtual bool put(
                Sample & sample, 
                boost::system::error_code & ec);

            virtual bool put(
                MuxEvent const & event, 
                boost::system::error_code & ec);

        public:
            static bool put_eof(
                Filter * filter, 
                boost::system::error_code & ec);

            static void detach(
                Filter * filter);

        private:
            Filter * filter_;
            MergeHook * hook_;
        };

    } // namespace mux
} // namespace ppbox

#endif // End _PPBOX_MUX_FILTER_MERGE_FILTER_H_
