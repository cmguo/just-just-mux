// Filter.h

#ifndef   _PPBOX_MUX_FILTER_H_
#define   _PPBOX_MUX_FILTER_H_

#include "ppbox/mux/MuxBase.h"

#include <ppbox/demux/base/BufferDemuxer.h>

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
            virtual boost::system::error_code open(
                MediaFileInfo const & media_file_info, 
                boost::system::error_code & ec)
            {
                return prev()->open(media_file_info, ec);
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
        };

        class DemuxFilter
            : public Filter
        {
        public:
            DemuxFilter()
                : demuxer_(NULL)
            {
            }

            ~DemuxFilter()
            {
                demuxer_ = NULL;
            }

            virtual boost::system::error_code open(
                MediaFileInfo const & media_file_info, 
                boost::system::error_code & ec)
            {
                assert(demuxer_);
                ec.clear();
                return ec;
            }

            virtual boost::system::error_code get_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec)
            {
                assert(demuxer_);
                return demuxer_->get_sample(sample, ec);
            }

            void set_demuxer(
                ppbox::demux::BufferDemuxer * demuxer)
            {
                demuxer_ = demuxer;
            }

        private:
            ppbox::demux::BufferDemuxer * demuxer_;
        };

    }
}

#endif
