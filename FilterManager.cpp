// FilterManager.cpp

#include "just/mux/Common.h"
#include "just/mux/FilterManager.h"
#include "just/mux/MuxError.h"
#include "just/mux/filter/MergeFilter.h"
#include "just/mux/filter/LastFilter.h"

using namespace just::avformat::error;

#include <just/demux/base/DemuxerBase.h>
#include <just/demux/base/DemuxError.h>

#include <algorithm>

namespace just
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
            just::demux::DemuxerBase * demuxer, 
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
            for (size_t i = 0; i < streams_.size(); ++i) {
                FilterPipe & pipe = this->pipe(i);
                pipe.insert(new MergeFilter(filter, adopt));
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
                        if ((sample.flags & sample.f_config)) {
                            StreamInfo & stream = *streams_[sample.itrack].info;
                            demuxer_->get_stream_info(sample.itrack, stream, ec);
                            FilterPipe & pipe = this->pipe(sample.itrack);
                            if (!pipe.put(stream, ec)) {
                                return false;
                            }
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

        bool FilterManager::before_seek(
            boost::uint64_t time, 
            boost::system::error_code & ec)
        {
            ec.clear();
            for (size_t i = 0; i < streams_.size(); ++i) {
                if (!pipe(i).put(MuxEvent(MuxEvent::before_seek, i, time), ec))
                    return false;
            }
            return !ec;
        }

        bool FilterManager::before_reset(
            boost::system::error_code & ec)
        {
            for (size_t i = 0; i < streams_.size(); ++i) {
                if (!pipe(i).put(MuxEvent(MuxEvent::before_reset, i), ec))
                    return false;
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

        bool FilterManager::after_reset(
            boost::system::error_code & ec)
        {
            ec.clear();
            is_eof_ = false;
            is_eof2_ = false;
            for (size_t i = 0; i < streams_.size(); ++i) {
                streams_[i].end = false;
                pipe(i).put(MuxEvent(MuxEvent::after_reset, i), ec);
            }
            return true;
        }

        bool FilterManager::after_seek(
            boost::uint64_t time, 
            boost::system::error_code & ec)
        {
            ec.clear();
            for (size_t i = 0; i < streams_.size(); ++i) {
                if (!pipe(i).put(MuxEvent(MuxEvent::after_seek, i, time), ec))
                    return false;
            }
            return true;
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
                if (!is_eof_) // this is fake end of segment filter
                    return true;
                streams_[event.itrack].end = true;
                is_eof2_ = std::find_if(streams_.begin(), streams_.end(), FilterStream::not_end()) == streams_.end();
            }
            ec.clear();
            return true;
        }

    } // namespace mux
} // namespace just
