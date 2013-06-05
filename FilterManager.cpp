// FilterManager.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/FilterManager.h"
#include "ppbox/mux/MuxError.h"
#include "ppbox/mux/filter/MergeFilter.h"
#include "ppbox/mux/filter/LastFilter.h"

#include <ppbox/demux/base/DemuxerBase.h>
#include <ppbox/demux/base/DemuxError.h>

namespace ppbox
{
    namespace mux
    {

        FilterManager::FilterManager()
            : demuxer_(NULL)
            , streams_(NULL)
            , is_save_sample_(false)
            , is_eof_(false)
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
            assert(filters_.empty());
            for (boost::uint32_t i = 0; i < stream_count; ++i) {
                filters_.push_back(new FilterPipe);
            }
            ec.clear();
            return true;
        }

        bool FilterManager::append_filter(
            Filter * filter, 
            boost::system::error_code & ec)
        {
            for (size_t i = 0; i < filters_.size(); ++i) {
                FilterPipe & pipe = *filters_[i];
                pipe.push_back(new MergeFilter(filter));
            }
            ec.clear();
            return true;
        }

        bool FilterManager::append_filter(
            boost::uint32_t stream, 
            Filter * filter, 
            boost::system::error_code & ec)
        {
            FilterPipe & pipe = *filters_[stream];
            pipe.push_back(filter);
            ec.clear();
            return true;
        }

        bool FilterManager::remove_filter(
            Filter * filter, 
            boost::system::error_code & ec)
        {
            MergeFilter::detach(filter);
            ec.clear();
            return true;
        }

        bool FilterManager::complete(
            framework::configure::Config & conf, 
            std::vector<StreamInfo> & streams, 
            boost::system::error_code & ec)
        {
            streams_ = &streams.at(0);
            for (size_t i = 0; i < filters_.size(); ++i) {
                FilterPipe & pipe = *filters_[i];
                StreamInfo & stream = streams[i];
                pipe.push_back(new LastFilter(*this));
                pipe.first()->config(conf);
                pipe.first()->put(stream, ec);
            }
            ec.clear();
            return true;
        }

        bool FilterManager::pull_one(
            Sample & sample,
            boost::system::error_code & ec)
        {
            assert(demuxer_);

            if (!out_samples_.empty()) {
                sample = out_samples_.front();
                out_samples_.pop_front();
                ec.clear();
                return true;
            } else if (is_eof_) {
                ec = error::end_of_stream;
                return false;
            }

            while (true) {
                if (is_save_sample_) {
                    sample = sample_;
                    is_save_sample_ = false;
                } else {
                    demuxer_->get_sample(sample, ec);
                    if (ec) {
                        if (ec == ppbox::demux::error::no_more_sample) {
                            for (size_t i = 0; i < filters_.size(); ++i) {
                                FilterPipe & pipe = *filters_[i];
                                Filter::eos_t eos;
                                pipe.first()->put(eos, ec);
                            }
                            is_eof_ = true;
                            if (!out_samples_.empty()) {
                                sample = out_samples_.front();
                                out_samples_.pop_front();
                                return true;
                            } else {
                                ec = error::end_of_stream;
                                return false;
                            }
                        }
                        break;
                    } else {
                        sample.stream_info = streams_ + sample.itrack;
                    }
                }
                FilterPipe & pipe = *filters_[sample.itrack];
                if (pipe.first()->put(sample, ec)) {
                    sample = out_samples_.front();
                    out_samples_.pop_front();
                    return true;
                }
                if (ec == error::need_more_sample) {
                    continue;
                }
                sample_ = sample;
                is_save_sample_ = true;
                return false;
            }

            return !ec;
        }

        bool FilterManager::reset(
            Sample & sample,
            boost::system::error_code & ec)
        {
            ec.clear();
            for (size_t i = 0; i < out_samples_.size(); ++i) {
                sample.append(out_samples_[i]);
            }
            out_samples_.clear();
            is_eof_ = false;
            for (size_t i = 0; i < filters_.size(); ++i) {
                FilterPipe & pipe = *filters_[i];
                pipe.first()->reset(sample, ec);
            }
            if (is_save_sample_) {
                sample.append(sample_);
                is_save_sample_ = false;
            }
            return demuxer_->free_sample(sample, ec);
        }

        bool FilterManager::close(
            boost::system::error_code & ec)
        {
            for (size_t i = 0; i < filters_.size(); ++i) {
                FilterPipe & pipe = *filters_[i];
                while (!pipe.empty()) {
                    delete pipe.last();
                }
                delete &pipe;
            }
            filters_.clear();
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


    } // namespace mux
} // namespace ppbox
