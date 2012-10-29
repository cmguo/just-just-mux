// DemuxerFilter.h

#ifndef _PPBOX_MUX_FILTER_DEMUXER_FILTER_H_
#define _PPBOX_MUX_FILTER_DEMUXER_FILTER_H_

#include "ppbox/mux/Filter.h"

#include <ppbox/demux/base/SegmentDemuxer.h>

namespace ppbox
{
    namespace mux
    {

        class DemuxerFilter
            : public Filter
        {
        public:
            DemuxerFilter()
                : demuxer_(NULL)
            {
            }

            ~DemuxerFilter()
            {
                demuxer_ = NULL;
            }

            virtual boost::system::error_code open(
                MediaInfo const & media_info, 
                std::vector<StreamInfo> const & streams, 
                boost::system::error_code & ec)
            {
                assert(demuxer_);
                ec.clear();
                return ec;
            }

            virtual boost::system::error_code get_sample(
                Sample & sample,
                boost::system::error_code & ec)
            {
                assert(demuxer_);
                return demuxer_->get_sample(sample, ec);
            }

            void set_demuxer(
                ppbox::demux::SegmentDemuxer * demuxer)
            {
                demuxer_ = demuxer;
            }

        private:
            ppbox::demux::SegmentDemuxer * demuxer_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FILTER_DEMUXER_FILTER_H_
