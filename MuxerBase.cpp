// Muxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/Transfer.h"
#include "ppbox/mux/MuxError.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"

#include <ppbox/demux/base/SegmentDemuxer.h>
#include <ppbox/demux/base/DemuxError.h>

#include <ppbox/avformat/codec/avc/AvcCodec.h>
using namespace ppbox::avformat;

#include <util/buffers/BuffersSize.h>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        std::map< std::string, MuxerBase::register_type > & MuxerBase::muxer_map()
        {
            static std::map< std::string, MuxerBase::register_type > get_map;
            return get_map;
        }

        void MuxerBase::register_muxer(
            std::string const & format, 
            register_type func)
        {
            muxer_map().insert(std::make_pair(format, func));
            return;
        }

        MuxerBase * MuxerBase::create(
            std::string const & format)
        {
            std::map< std::string, register_type >::iterator iter = muxer_map().find(format);
            if (muxer_map().end() == iter) {
                return NULL;
            }
            MuxerBase * muxer = iter->second();
            muxer->format_ = format;
            return muxer;
        }

        void MuxerBase::destory(
            MuxerBase* & muxer)
        {
            delete muxer;
            muxer = NULL;
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
            demux::SegmentDemuxer * demuxer,
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
            }
            return !ec;
        }

        bool MuxerBase::setup(
            boost::uint32_t index, 
            boost::system::error_code & ec)
        {
            if (index != -1) {
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
                    boost::uint64_t time = demuxer_->get_cur_time(ec);
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
                boost::uint64_t time = demuxer_->get_cur_time(ec);
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
            demuxer_->media().get_info(info, ec);
            info.file_size = ppbox::data::invalid_size;
            info.format = format_;
        }

        void MuxerBase::play_info(
            PlayInfo & info) const
        {
            MediaInfo info1;
            media_info(info1);
            info.byte_range.beg = 0;
            info.byte_range.pos = 0;
            info.byte_range.end = info1.file_size;
            info.time_range.beg = seek_time_;
            info.time_range.pos = play_time_;
            info.time_range.end = info1.duration;
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
                if (stream.type == MEDIA_TYPE_VIDE 
                    && stream.sub_type == VIDEO_TYPE_AVC1) {
                        stream.codec = new AvcCodec(stream.format_data);
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
            for (boost::uint32_t i = 0; i < streams_.size(); ++i) {
                if (streams_[i].codec) {
                    delete streams_[i].codec;
                }
                for (boost::uint32_t j = 0; j < transfers_[i].size(); ++j) {
                    delete transfers_[i][j];
                }
            }
            streams_.clear();
            transfers_.clear();
        }

    } // namespace mux
} // namespace ppbox
