// FilterManager.h

#ifndef _PPBOX_MUX_FILTER_MANAGER_H_
#define _PPBOX_MUX_FILTER_MANAGER_H_

#include "ppbox/mux/Filter.h"
#include "ppbox/mux/FilterPipe.h"

#include <ppbox/demux/base/DemuxerBase.h>

namespace ppbox
{
    namespace mux
    {

        class MergeHook;
        class LastFilter;

        class FilterManager
        {
        public:
            FilterManager();

            ~FilterManager();

        public:
            bool open(
                ppbox::demux::DemuxerBase * demuxer, 
                boost::uint32_t stream_count, 
                boost::system::error_code & ec);

            bool append_filter(
                Filter * filter, 
                bool adopt, 
                boost::system::error_code & ec);

            bool append_filter(
                boost::uint32_t stream, 
                Filter * filter, 
                bool adopt, 
                boost::system::error_code & ec);

            bool remove_filter(
                Filter * filter, 
                boost::system::error_code & ec);

            bool complete(
                framework::configure::Config & conf, 
                std::vector<StreamInfo> & streams, 
                boost::system::error_code & ec);

            bool pull_one(
                Sample & sample,
                boost::system::error_code & ec);

            bool begin_reset(
                boost::system::error_code & ec);

            bool begin_seek(
                boost::uint64_t time, 
                boost::system::error_code & ec);

            bool finish_seek(
                boost::uint64_t time, 
                boost::system::error_code & ec);

            bool reset(
                boost::system::error_code & ec);

            bool close(
                boost::system::error_code & ec);

        public:
            FilterPipe & pipe(
                boost::uint32_t index)
            {
                return *streams_[index].pipe;
            }

        private:
            friend class LastFilter;

            bool put(
                Sample & sample,
                boost::system::error_code & ec);

            bool put(
                MuxEvent const & event, 
                boost::system::error_code & ec);

        private:
            struct FilterStream
            {
                FilterStream()
                    : info(NULL)
                    , pipe(NULL)
                    , end(false)
                {
                }

                struct not_end;

                StreamInfo * info;
                FilterPipe * pipe;
                bool end;
            };

            ppbox::demux::DemuxerBase * demuxer_;
            std::vector<FilterStream> streams_;

            bool is_save_sample_;
            Sample sample_;

            std::deque<Sample> out_samples_;
            bool is_eof_;
            bool is_eof2_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FILTER_MANAGER_H_