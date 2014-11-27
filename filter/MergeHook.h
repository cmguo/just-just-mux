// MergeHook.h

#ifndef _JUST_MUX_FILTER_MERGE_HOOK_H_
#define _JUST_MUX_FILTER_MERGE_HOOK_H_

#include "just/mux/Filter.h"

namespace just
{
    namespace mux
    {

        class MergeFilter;

        class MergeHook
            : public Filter
        {
        public:
            MergeHook()
                : next_(NULL)
            {
            }

        public:
            virtual void config(
                framework::configure::Config & conf)
            {
                next_->config(conf);
            }

        public:
            virtual bool put(
                StreamInfo & info, 
                boost::system::error_code & ec)
            {
                return next_->put(info, ec);
            }

            virtual bool put(
                Sample & sample, 
                boost::system::error_code & ec)
            {
                return next_->put(sample, ec);
            }

            virtual bool put(
                MuxEvent const & event, 
                boost::system::error_code & ec)
            {
                return next_->put(event, ec);
            }

        private:
            friend class MergeFilter;

            void wrapper(
                Filter * filter)
            {
                wrappers_.push_back(filter);
            }

            void unwrapper(
                Filter * filter)
            {
                wrappers_.erase(
                    std::find(wrappers_.begin(), wrappers_.end(), filter));
            }

            void current(
                Filter * filter)
            {
                next_ = filter->next();
            }

            bool put_eof(
                boost::system::error_code & ec)
            {
                for (size_t i = 0; i < wrappers_.size(); ++i) {
                    wrappers_[i]->next()->put(MuxEvent(MuxEvent::end, i), ec);
                }
                return true;
            }

            void detach_wrappers()
            {
                while (!wrappers_.empty()) {
                    wrappers_[0]->unlink();
                    wrappers_[0]->detach();
                }
            }

        private:
            std::vector<Filter *> wrappers_;
            Filter * next_;
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_FILTER_MERGE_HOOK_H_
