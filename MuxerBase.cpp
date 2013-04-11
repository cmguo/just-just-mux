// Muxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/Transfer.h"
#include "ppbox/mux/MuxError.h"
#include "ppbox/mux/filter/DemuxerFilter.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"

#include <ppbox/demux/base/DemuxerBase.h>
#include <ppbox/demux/base/DemuxError.h>

#include <ppbox/avformat/codec/Codec.h>
using namespace ppbox::avformat;

#include <util/buffers/BuffersSize.h>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        MuxerBase * MuxerBase::create(
            std::string const & format)
        {
            MuxerBase * muxer = factory_type::create(format);
            if (muxer) {
                muxer->format_ = format;
            }
            return muxer;
        }

        MuxerBase::MuxerBase()
            : demuxer_(NULL)
            , read_flag_(0)
            , head_step_(0)
        {
            demux_filter_ = new DemuxerFilter;
            key_filter_ = new KeyFrameFilter;
        }

        MuxerBase::~MuxerBase()
        {
            if (demuxer_ != NULL) {
                // demuxer具体的析构不在mux内里实现
                demuxer_ = NULL;
            }
            delete key_filter_;
            delete demux_filter_;
        }

        bool MuxerBase::open(
            ppbox::demux::DemuxerBase * demuxer,
            error_code & ec)
        {
            assert(demuxer != NULL);
            demuxer_ = demuxer;
            demux_filter_->set_demuxer(demuxer_);
            filters_.push_back(demux_filter_);
            filters_.push_back(key_filter_);
            open(ec);
            if (ec) {
                demuxer_ = NULL;
            } else {
                read_flag_ = f_head;
                stat_.time_range.beg = stat_.time_range.pos = demuxer_->check_seek(ec);
                if (ec == boost::asio::error::would_block) {
                    ec.clear();
                    read_flag_ = f_seek;
                }
            }
            return !ec;
        }

        bool MuxerBase::setup(
            boost::uint32_t index, 
            boost::system::error_code & ec)
        {
            if (index != (boost::uint32_t)-1) {
                ec == framework::system::logic_error::not_supported;
            } else {
                ec.clear();
            }
            return !ec;
        }

        bool MuxerBase::read(
            Sample & sample,
            error_code & ec)
        {
            if (read_flag_) {
                if (read_flag_ & f_head) {
                    sample.itrack = (boost::uint32_t)-1;
                    sample.time = 0;
                    sample.ustime = 0;
                    sample.dts = 0;
                    sample.us_delta = 0;
                    sample.cts_delta = 0;
                    sample.duration = 0;
                    sample.size = 0;
                    sample.stream_info = NULL;
                    sample.data.clear();
                    do {
                        if (head_step_ == 0) {
                            file_header(sample);
                            ++head_step_;
                        } else {
                            assert(head_step_ <= streams_.size());
                            stream_header(head_step_ - 1, sample);
                            if (head_step_ == streams_.size()) {
                                head_step_ = 0;
                                read_flag_ &= ~f_head;
                            } else {
                                ++head_step_;
                            }
                        }
                    } while (sample.data.empty() && (read_flag_ & f_head));
                    if (!sample.data.empty()) {
                        sample.size = util::buffers::buffers_size(sample.data);
                        ec.clear();
                        stat_.byte_range.pos += sample.size;
                        return true;
                    }
                } else if (read_flag_ & f_seek) {
                    boost::uint64_t time = demuxer_->check_seek(ec);
                    if (!ec) {
                        on_seek(time);
                        read_flag_ &= ~f_seek;
                    } else {
                        return false;
                    }
                }
            }

            get_sample(sample, ec);

            return !ec;
        }

        bool MuxerBase::reset(
            error_code & ec)
        {
            if (!before_seek(true, 0, ec)) {
                return false;
            }
            demuxer_->reset(ec);
            if (!ec) {
                read_flag_ |= f_head;
                boost::uint64_t time = demuxer_->check_seek(ec);
                if (!ec) {
                    on_seek(time);
                }
            } else if (ec ==boost::asio::error::would_block) {
                read_flag_ |= f_head;
                read_flag_ |= f_seek;
            }
            return !ec;
        }

        bool MuxerBase::time_seek(
            boost::uint64_t & time,
            error_code & ec)
        {
            if (!before_seek(false, time, ec)) {
                return false;
            }
            demuxer_->seek(time, ec);
            if (!ec) {
                read_flag_ |= f_head;
                on_seek(time);
            } else if (ec == boost::asio::error::would_block) {
                stat_.time_range.beg = time;
                read_flag_ |= f_head;
                read_flag_ |= f_seek;
            }
            stat_.byte_range.beg = 0;
            stat_.byte_range.end = ppbox::data::invalid_size;
            stat_.byte_range.pos = 0;
            stat_.byte_range.buf = 0;
            return !ec;
        }

        bool MuxerBase::byte_seek(
            boost::uint64_t & offset,
            boost::system::error_code & ec)
        {
            boost::uint64_t seek_time = (offset * media_info_.duration) / media_info_.file_size;
            return time_seek(seek_time, ec);
        }

        boost::uint64_t MuxerBase::check_seek(
            boost::system::error_code & ec)
        {
            if (read_flag_ & f_seek) {
                boost::uint64_t time = demuxer_->check_seek(ec);
                if (!ec) {
                    on_seek(time);
                    read_flag_ &= ~f_seek;
                }
            } else {
                ec.clear();
            }
            return stat_.time_range.beg;
        }

        void MuxerBase::media_info(
            MediaInfo & info) const
        {
            boost::system::error_code ec;
            demuxer_->get_media_info(info, ec);
            info.file_size = ppbox::data::invalid_size;
            info.format = format_;
        }

        void MuxerBase::stream_info(
            std::vector<StreamInfo> & streams) const
        {
            streams = streams_;
        }

        void MuxerBase::stream_status(
            StreamStatus & status) const
        {
            boost::system::error_code ec;
            StreamStatus status1;
            demuxer_->get_stream_status(status1, ec);
            status1.byte_range = stat_.byte_range;
            status1.time_range.beg = stat_.time_range.beg;
            status = status1;
        }

        bool MuxerBase::close(
            boost::system::error_code & ec)
        {
            close();
            demuxer_ = NULL;
            ec.clear();
            return true;
        }

        void MuxerBase::add_filter(
            Filter * filter)
        {
            filters_.push_back(filter);
        }

        void MuxerBase::reset_header(
            bool file_header, 
            bool stream_header)
        {
            read_flag_ |= f_head;
            head_step_ = file_header ? 0 : 1;
        }

        void MuxerBase::open(
                error_code & ec)
        {
            assert(demuxer_ != NULL);
            demuxer_->get_media_info(media_info_, ec);
            if (ec) {
                return;
            }
            do_open(media_info_);
            size_t stream_count = demuxer_->get_stream_count(ec);
            transfers_.clear();
            transfers_.resize(stream_count);
            for (size_t i = 0; i < stream_count; ++i) {
                StreamInfo stream;
                demuxer_->get_stream_info(i, stream, ec);
                if (ec) {
                    break;
                }
                // TODO: add codec
                if (stream.codec == NULL) {
                    stream.codec = Codec::create(stream.sub_type, stream.format_data);
                }
                add_stream(stream, transfers_[i]);
                streams_.push_back(stream);
                for(boost::uint32_t j = 0; j < transfers_[i].size(); ++j) {
                    transfers_[i][j]->config(config_);
                    transfers_[i][j]->transfer(stream);
                }
            }
            if (!ec) {
                filters_.last()->open(media_info_, streams_, ec);
            }
        }

        bool MuxerBase::before_seek(
            bool reset, 
            boost::uint64_t time,
            boost::system::error_code & ec)
        {
            Sample sample;
            sample.time = time;
            for(size_t i = 0; i < transfers_.size(); ++i) {
                std::vector<Transfer *> & transfers = transfers_[i];
                for(boost::uint32_t j = 0; j < transfers.size(); ++j) {
                    transfers[j]->before_seek(sample);
                }
            }
            sample.flags = reset ? sample.sync : 0;
            return filters_.last()->before_seek(sample, ec);
        }

        void MuxerBase::on_seek(
            boost::uint64_t time)
        {
            stat_.time_range.beg = time;
            stat_.time_range.pos = time;
            filters_.last()->on_seek(time);
            for (boost::uint32_t i = 0; i < transfers_.size(); ++i) {
                for (boost::uint32_t j = 0; j < transfers_[i].size(); ++j) {
                    transfers_[i][j]->on_seek(time);
                }
            }
        }

        void MuxerBase::get_sample(
            Sample & sample,
            error_code & ec)
        {
            filters_.last()->get_sample(sample, ec);
            if (!ec) {
                sample.stream_info = &streams_[sample.itrack];
                //if (sample.flags & Sample::stream_changed) {
                    //release_info();
                    //open_impl(ec);
                    //read_flag_ |= f_head;
                    //head_step_ = 1;
                //}
                stat_.time_range.pos = sample.time;
                std::vector<Transfer *> & transfers = transfers_[sample.itrack];
                for(boost::uint32_t i = 0; i < transfers.size(); ++i) {
                    transfers[i]->transfer(sample);
                }
                stat_.byte_range.pos += sample.size;
            } else if (ec == ppbox::demux::error::no_more_sample) {
                stat_.byte_range.end = stat_.byte_range.pos;
                ec = error::end_of_stream;
            }
        }

        void MuxerBase::close()
        {
            for (boost::uint32_t i = 0; i < transfers_.size(); ++i) {
                for (boost::uint32_t j = 0; j < transfers_[i].size(); ++j) {
                    delete transfers_[i][j];
                }
            }
            error_code ec;
            before_seek(true, 0, ec);
            demux_filter_->unlink();
            key_filter_->unlink();
            do_close();
            while (!filters_.empty()) {
                delete filters_.last();
            }
            streams_.clear();
            transfers_.clear();
        }

    } // namespace mux
} // namespace ppbox
