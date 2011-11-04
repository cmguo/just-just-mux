// BaseFilter.h

#ifndef   _PPBOX_MUX_BASE_FILTER_H_
#define   _PPBOX_MUX_BASE_FILTER_H_

#include "ppbox/mux/MuxerBase.h"
#include <ppbox/demux/Demuxer.h>

namespace ppbox
{
    namespace mux
    {
        class BaseFilter
        {
        public:
            BaseFilter(ppbox::demux::Demuxer * demuxer)
                : demuxer_(demuxer)
            {
            }

            virtual ~BaseFilter()
            {
            }

            boost::system::error_code get_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec)
            {
                assert(demuxer_ != NULL);
                demuxer_->get_sample_buffered(sample);
                return ec;
            }

        private:
            ppbox::demux::Demuxer * demuxer_;

        };
    }
}

#endif // End _PPBOX_MUX_BASE_FILTER_H_
