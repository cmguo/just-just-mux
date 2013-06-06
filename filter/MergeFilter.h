// MergeFilter.h

#ifndef _PPBOX_MUX_FILTER_MERGE_FILTER_H_
#define _PPBOX_MUX_FILTER_MERGE_FILTER_H_

#include "ppbox/mux/Filter.h"
#include "ppbox/mux/filter/MergeHook.h"

namespace ppbox
{
    namespace mux
    {

        class MergeFilter
            : public Filter
        {
        public:
            MergeFilter(
                Filter * filter)
                : filter_(filter)
            {
                if (filter_->is_linked()) {
                    hook_ = (MergeHook *)filter_->next();
                } else {
                    hook_ = new MergeHook;
                    filter->insert(filter, hook_);
                }
                hook_->wrapper(this);
            }

            ~MergeFilter()
            {
                hook_->unwrapper(this);
            }

        public:
            virtual void config(
                framework::configure::Config & conf)
            {
                hook_->current(this);
                filter_->config(conf);
            }

        public:
            virtual bool put(
                StreamInfo & info, 
                boost::system::error_code & ec)
            {
                hook_->current(this);
                return filter_->put(info, ec);
            }

            virtual bool put(
                Sample & sample, 
                boost::system::error_code & ec)
            {
                hook_->current(this);
                return filter_->put(sample, ec);
            }

            virtual bool put(
                MuxEvent const & event, 
                boost::system::error_code & ec)
            {
                hook_->current(this);
                return filter_->put(event, ec);
            }

        public:
            static bool put_eof(
                Filter * filter, 
                boost::system::error_code & ec)
            {
                assert(filter->is_linked());
                MergeHook * hook = (MergeHook *)filter->next();
                return hook->put_eof(ec);
            }

            static void detach(
                Filter * filter)
            {
                MergeHook * hook = (MergeHook *)filter->next();
                filter->unlink();
                hook->detach();
            }

        private:
            Filter * filter_;
            MergeHook * hook_;
        };

    } // namespace mux
} // namespace ppbox

#endif // End _PPBOX_MUX_FILTER_MERGE_FILTER_H_
