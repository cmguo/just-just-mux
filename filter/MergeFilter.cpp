// MergeFilter.cpp

#include "just/mux/Common.h"
#include "just/mux/filter/MergeFilter.h"
#include "just/mux/filter/MergeHook.h"

namespace just
{
    namespace mux
    {

        MergeFilter::MergeFilter(
            Filter * filter, 
            bool adopt)
            : filter_(filter)
        {
            if (filter_->is_linked()) {
                filter_->attach();
                hook_ = (MergeHook *)filter_->next();
                hook_->attach();
            } else {
                hook_ = new MergeHook;
                filter_->insert(filter_, hook_);
                if (!adopt)
                    filter_->attach();
            }
            hook_->wrapper(this);
        }

        MergeFilter::~MergeFilter()
        {
            hook_->unwrapper(this);
            hook_->detach();
            filter_->detach();
        }

        void MergeFilter::config(
            framework::configure::Config & conf)
        {
            hook_->current(this);
            filter_->config(conf);
        }

        bool MergeFilter::put(
            StreamInfo & info, 
            boost::system::error_code & ec)
        {
            hook_->current(this);
            return filter_->put(info, ec);
        }

        bool MergeFilter::put(
            Sample & sample, 
            boost::system::error_code & ec)
        {
            hook_->current(this);
            return filter_->put(sample, ec);
        }

        bool MergeFilter::put(
            MuxEvent const & event, 
            boost::system::error_code & ec)
        {
            hook_->current(this);
            return filter_->put(event, ec);
        }

        bool MergeFilter::put_eof(
            Filter * filter, 
            boost::system::error_code & ec)
        {
            assert(filter->is_linked());
            MergeHook * hook = (MergeHook *)filter->next();
            return hook->put_eof(ec);
        }

        void MergeFilter::detach(
            Filter * filter)
        {
            MergeHook * hook = (MergeHook *)filter->next();
            // need add ref when we use this hook
            hook->attach();
            hook->detach_wrappers();
            hook->detach();
        }

    } // namespace mux
} // namespace just
