// Filter.h

#ifndef   _PPBOX_MUX_FILTER_H_
#define   _PPBOX_MUX_FILTER_H_

#include "ppbox/mux/MuxerBase.h"
#include <ppbox/demux/pptv/PptvDemuxer.h>

#include <framework/container/List.h>

namespace ppbox
{
    namespace mux
    {

        class Filter
            : public framework::container::ListHook<Filter>::type
        {
        public:
            Filter(
                MediaFileInfo const & media_file_info)
                : media_file_info_(media_file_info)
            {
            }

            virtual ~Filter()
            {
            }

            virtual boost::system::error_code get_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec)
            {
                return prev()->get_sample(sample, ec);
            }

        protected:
            void detach_self()
            {
                unlink();
            }

            MediaFileInfo const & media_file_info(void)
            {
                return media_file_info_;
            }

        private:
            MediaFileInfo const & media_file_info_;
        };

        class DemuxFilter
            : public Filter
        {
        public:
            DemuxFilter(
                MediaFileInfo const & media_file_info)
                : Filter(media_file_info)
                , demuxer_(NULL)
            {
            }

            ~DemuxFilter()
            {
                demuxer_ = NULL;
            }

            virtual boost::system::error_code get_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec)
            {
                assert(demuxer_);
                return demuxer_->get_sample(sample, ec);
            }

            void set_demuxer(ppbox::demux::PptvDemuxer * demuxer)
            {
                demuxer_ = demuxer;
            }

        private:
            ppbox::demux::PptvDemuxer * demuxer_;

        };

    }
}

#endif
