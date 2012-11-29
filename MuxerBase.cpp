// Muxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/Transfer.h"
#include "ppbox/mux/MuxError.h"
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
            framework::string::Url url_format("mux:///" + format);
            std::string foramt1 = url_format.path().substr(1);
            MuxerBase * muxer = factory_type::create(foramt1);
            if (muxer) {
                muxer->format_ = foramt1;
                framework::string::Url::param_const_iterator iter = url_format.param_begin();
                for (; iter != url_format.param_end(); ++iter) {
                    std::string key = iter->key();
                    std::string::size_type pos_dot = key.rfind('.');
                    if (pos_dot == std::string::npos)
                        continue;
                    muxer->config().set(
                        key.substr(0, pos_dot), 
                        key.substr(pos_dot + 1), 
                        iter->value());
                }
            }
            return muxer;
        }

        MuxerBase::MuxerBase()
            : demuxer_(NULL)
            , seek_time_(0)
            , play_time_(0)
            , read_flag_(0)
            , head_step_(0)
        {
            filters_.push_back(&demux_filter_);
            filters_.push_back(&key_filter_);
        }

        MuxerBase::~MuxerBase()
        {
            if (demuxer_ != NULL) {
                // demuxer具体的析构不在mux内里实现
                demuxer_ = NULL;
            }
        }

        bool MuxerBase::open(
            ppbox::demux::DemuxerBase * demuxer,
            error_code & ec)
        {
            assert(demuxer != NULL);
            demuxer_ = demuxer;
            demux_filter_.set_demuxer(demuxer_);
            open(ec);
            if (ec) {
                demuxer_ = NULL;
            } else {
                read_flag_ = f_head;
                seek_time_ = play_time_ = demuxer_->check_seek(ec);
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
                    do {
                        sample.data.clear();
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
            demuxer_->seek(time, ec);
            if (!ec) {
                read_flag_ |= f_head;
                on_seek(time);
            } else if (ec ==boost::asio::error::would_block) {
                read_flag_ |= f_head;
                read_flag_ |= f_seek;
            }
            return !ec;
        }

        bool MuxerBase::byte_seek(
            boost::uint64_t & offset,
            boost::system::error_code & ec)
        {
            boost::uint64_t seek_time = (offset * media_info_.duration) / media_info_.file_size;
            return time_seek(seek_time, ec);
        }

        void MuxerBase::media_info(
            MediaInfo & info) const
        {
            boost::system::error_code ec;
            demuxer_->get_media_info(info, ec);
            info.file_size = ppbox::data::invalid_size;
            info.format = format_;
        }

        void MuxerBase::stream_status(
            StreamStatus & status) const
        {
            boost::system::error_code ec;
            demuxer_->get_stream_status(status, ec);
        }

        bool MuxerBase::close(
            boost::system::error_code & ec)
        {
            close();
            demuxer_ = NULL;
            ec.clear();
            return true;
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
                    if (stream.codec) {
                        codecs_.push_back(stream.codec);
                    }
                }
                add_stream(stream, transfers_[i]);
                streams_.push_back(stream);
                for(boost::uint32_t j = 0; j < transfers_[i].size(); ++j) {
                    transfers_[i][j]->transfer(stream);
                }
            }
            if (!ec) {
                filters_.last()->open(media_info_, streams_, ec);
            }
        }

        void MuxerBase::on_seek(
            boost::uint64_t time)
        {
            seek_time_ = time;
            play_time_ = time;
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
                play_time_ = sample.time;
                std::vector<Transfer *> & transfers = transfers_[sample.itrack];
                for(boost::uint32_t i = 0; i < transfers.size(); ++i) {
                    transfers[i]->transfer(sample);
                }
            } else if (ec == ppbox::demux::error::no_more_sample) {
                ec = error::end_of_stream;
            }
        }

        void MuxerBase::close()
        {
            for (boost::uint32_t i = 0; i < codecs_.size(); ++i) {
                delete codecs_[i];
            }
            for (boost::uint32_t i = 0; i < transfers_.size(); ++i) {
                for (boost::uint32_t j = 0; j < transfers_[i].size(); ++j) {
                    delete transfers_[i][j];
                }
            }
            streams_.clear();
            transfers_.clear();
        }

    } // namespace mux
} // namespace ppbox
