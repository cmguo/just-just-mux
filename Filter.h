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
            virtual ~Filter()
            {
            }

        public:
            virtual bool open(
                MediaInfo const & media_info, 
                std::vector<StreamInfo> const & streams, 
                boost::system::error_code & ec)
            {
                return prev()->open(media_info, streams, ec);
            }

            virtual bool get_sample(
                Sample & sample, 
                boost::system::error_code & ec)
            {
                return prev()->get_sample(sample, ec);
            }

            virtual bool before_seek(
                Sample & sample, 
                boost::system::error_code & ec)
            {
                return prev()->before_seek(sample, ec);
            }

            virtual void on_seek(
                boost::uint64_t time)
            {
                prev()->on_seek(time);
            }

        protected:
            void detach_self()
            {
                unlink();
            }
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FILTER_FILTER_H_
