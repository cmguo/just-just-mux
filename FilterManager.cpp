// FilterManager.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/FilterManager.h"
#include "ppbox/mux/MuxError.h"
#include "ppbox/mux/filter/MergeFilter.h"
#include "ppbox/mux/filter/LastFilter.h"

using namespace ppbox::avformat::error;

#include <ppbox/demux/base/DemuxerBase.h>
#include <ppbox/demux/base/DemuxError.h>

#include <algorithm>

namespace ppbox
{
    namespace mux
    {

        FilterManager::FilterManager()
            : demuxer_(NULL)
            , streams_(NULL)
            , is_save_sample_(false)
            , is_eof_(false)
            , is_eof2_(false)
        {
        }

        FilterManager::~FilterManager()
        {
            demuxer_ = NULL;
        }

        bool FilterManager::open(
            ppbox::demux::DemuxerBase * demuxer, 
            boost::uint32_t stream_count, 
            boost::system::error_code & ec)
        {
            demuxer_ = demuxer;
            assert(streams_.empty());
            for (boost::uint32_t i = 0; i < stream_count; ++i) {
                FilterStream stream;
                stream.pipe = new FilterPipe;
                streams_.push_back(stream);
            }
            ec.clear();
            return true;
        }

        bool FilterManager::append_filter(
            Filter * filter, 
            bool adopt, 
            boost::system::error_code & ec)
        {
            if (!adopt) {
                filter->attach();
            }
            for (size_t i = 0; i < streams_.size(); ++i) {
                FilterPipe & pipe = this->pipe(i);
                pipe.insert(new MergeFilter(filter));
            }
            ec.clear();
            return true;
        }

        bool FilterManager::append_filter(
            boost::uint32_t stream, 
            Filter * filter, 
            bool adopt, 
            boost::system::error_code & ec)
        {
            FilterPipe & pipe = this->pipe(stream);
            pipe.insert(filter, adopt);
            ec.clear();
            return true;
        }

        bool FilterManager::remove_filter(
            Filter * filter, 
            boost::system::error_code & ec)
        {
            if (!filter->is_linked()) {
                return false;
            }
            MergeFilter::detach(filter);
            ec.clear();
            return true;
        }

        bool FilterManager::complete(
            framework::configure::Config & conf, 
            std::vector<StreamInfo> & streams, 
            boost::system::error_code & ec)
        {
            for (size_t i = 0; i < streams_.size(); ++i) {
                StreamInfo & stream = streams[i];
                streams_[i].info = &stream;
                FilterPipe & pipe = this->pipe(i);
                pipe.insert(new LastFilter(*this));
                pipe.config(conf);
                pipe.put(stream, ec);
            }
            ec.clear();
            return true;
        }

        bool FilterManager::pull_one(
            Sample & sample,
            boost::system::error_code & ec)
        {
            assert(demuxer_);

            while (out_samples_.empty()) {
                if (is_eof_) {
                    if (is_eof2_) {
                        ec = end_of_stream;
                        return false;
                    }
                    for (size_t i = 0; i < streams_.size(); ++i) {
                        if (!streams_[i].end) {
                            pipe(i).put(MuxEvent(MuxEvent::end, i), ec);
                        }
                    }
                } else {
                    if (is_save_sample_) {
                        sample = sample_;
                        is_save_sample_ = false;
                    } else {
                        demuxer_->get_sample(sample, ec);
                        if (ec) {
                            if (ec == end_of_stream) {
                                is_eof_ = true;
                                continue;
                            }
                            // get failed, return
                            return false;
                        }
                        sample.stream_info = streams_[sample.itrack].info;
                    }
                    FilterPipe & pipe = this->pipe(sample.itrack);
                    if (!pipe.put(sample, ec)) {
                        // put failed, return
                        sample_ = sample;
                        is_save_sample_ = true;
                        return false;
                    }
               }
            }

            sample = out_samples_.front();
            out_samples_.pop_front();
            return true;
        }

        bool FilterManager::begin_reset(
            boost::system::error_code & ec)
        {
            for (size_t i = 0; i < streams_.size(); ++i) {
                if (!pipe(i).put(MuxEvent(MuxEvent::begin_reset, i), ec))
                    return false;
            }
            return true;
        }

        bool FilterManager::begin_seek(
            boost::uint64_t time, 
            boost::system::error_code & ec)
        {
            for (size_t i = 0; i < streams_.size(); ++i) {
                if (!pipe(i).put(MuxEvent(MuxEvent::begin_seek, i, time), ec))
                    return false;
            }
            return true;
        }

        bool FilterManager::finish_seek(
            boost::uint64_t time, 
            boost::system::error_code & ec)
        {
            for (size_t i = 0; i < streams_.size(); ++i) {
                if (!pipe(i).put(MuxEvent(MuxEvent::finish_seek, i, time), ec))
                    return false;
            }
            return true;
        }

        bool FilterManager::reset(
            boost::system::error_code & ec)
        {
            ec.clear();
            is_eof_ = false;
            is_eof2_ = false;
            for (size_t i = 0; i < streams_.size(); ++i) {
                streams_[i].end = false;
                pipe(i).put(MuxEvent(MuxEvent::reset, i), ec);
            }
            Sample sample;
            if (is_save_sample_) {
                sample.append(sample_);
                is_save_sample_ = false;
            }
            for (size_t i = 0; i < out_samples_.size(); ++i) {
                sample.append(out_samples_[i]);
            }
            out_samples_.clear();
            return demuxer_->free_sample(sample, ec);
        }

        bool FilterManager::close(
            boost::system::error_code & ec)
        {
            for (size_t i = 0; i < streams_.size(); ++i) {
                delete streams_[i].pipe;
            }
            streams_.clear();
            ec.clear();
            return true;
        }

        bool FilterManager::put(
            Sample & sample,
            boost::system::error_code & ec)
        {
            out_samples_.push_back(sample);
            ec.clear();
            return true;
        }

        struct FilterManager::FilterStream::not_end
        {
            bool operator()(
                FilterManager::FilterStream const & s)
            {
                return !s.end;
            }
        };

        bool FilterManager::put(
            MuxEvent const & event, 
            boost::system::error_code & ec)
        {
            if (event.type == MuxEvent::end) {
                streams_[event.itrack].end = true;
                is_eof2_ = std::find_if(streams_.begin(), streams_.end(), FilterStream::not_end()) == streams_.end();
            }
            ec.clear();
            return true;
        }

    } // namespace mux
} // namespace ppbox
