// MergeHook.h

#ifndef _PPBOX_MUX_FILTER_MERGE_HOOK_H_
#define _PPBOX_MUX_FILTER_MERGE_HOOK_H_

#include "ppbox/mux/Filter.h"

namespace ppbox
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
                if (wrappers_.empty()) {
                    if (is_linked()) {
                        next()->unlink();
                        next()->detach();
                    }
                    delete this;
                }
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
                    wrappers_[i]->next()->put(MuxEvent::end, ec);
                }
                return true;
            }

            void detach()
            {
                while (!wrappers_.empty()) {
                    delete wrappers_[0];
                }
            }

        private:
            std::vector<Filter *> wrappers_;
            Filter * next_;
        };

    } // namespace mux
} // namespace ppbox

#endif // End _PPBOX_MUX_FILTER_MERGE_HOOK_H_
