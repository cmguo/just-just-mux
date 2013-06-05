// DemuxerFilter.h

#ifndef _PPBOX_MUX_FILTER_DEMUXER_FILTER_H_
#define _PPBOX_MUX_FILTER_DEMUXER_FILTER_H_

#include "ppbox/mux/Filter.h"

#include <ppbox/demux/base/DemuxerBase.h>

namespace ppbox
{
    namespace mux
    {

        class FilterManager
        {
        public:
            FilterManager()
                : demuxer_(NULL)
            {
            }

            ~FilterManager()
            {
                demuxer_ = NULL;
            }

        public:
            virtual bool open(
                MediaInfo const & media_info, 
                std::vector<StreamInfo> const & streams, 
                boost::system::error_code & ec)
            {
                assert(demuxer_);
                ec.clear();
                return true;
            }

            virtual bool get_sample(
                boost::system::error_code & ec)
            {
                assert(demuxer_);
                demuxer_->get_sample(sample, ec);
                return !ec;
            }

            virtual bool before_seek(
                Sample & sample,
                boost::system::error_code & ec)
            {
                return demuxer_->free_sample(sample, ec);
            }

            virtual void on_seek(
                boost::uint64_t time)
            {
            }

        public:
            void set_demuxer(
                ppbox::demux::DemuxerBase * demuxer)
            {
                demuxer_ = demuxer;
            }

        private:
            ppbox::demux::DemuxerBase * demuxer_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FILTER_DEMUXER_FILTER_H_
