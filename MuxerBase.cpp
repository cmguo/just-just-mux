// Muxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/Transfer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/rtp/RtpPacket.h"

#include <ppbox/demux/base/SegmentDemuxer.h>
#include <ppbox/demux/base/SourceError.h>
using namespace ppbox::demux;

#include <ppbox/data/MediaBase.h>

#include <ppbox/avformat/asf/AsfObjectType.h>
#include <ppbox/avformat/codec/avc/AvcCodec.h>
using namespace ppbox::avformat;

#include <util/buffers/BufferCopy.h>
#include <util/archive/ArchiveBuffer.h>

#include <framework/memory/MemoryPage.h>

#include <boost/thread/thread.hpp>
using namespace boost::system;

#include <iostream>

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
            , paused_(false)
            , play_time_(0)
            , read_step_(0)
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

        error_code MuxerBase::open(
            demux::SegmentDemuxer * demuxer,
            error_code & ec)
        {
            assert(demuxer != NULL);
            demuxer_ = demuxer;
            demux_filter_.set_demuxer(demuxer_);
            open_impl(ec);
            if (ec) {
                demuxer_ = NULL;
            }
            return ec;
        }

        bool MuxerBase::is_open()
        {
            if (demuxer_) {
                return true;
            } else {
                return false;
            }
        }

        error_code MuxerBase::open_impl(
            error_code & ec)
        {
            assert(demuxer_ != NULL);
            demuxer_->get_media_info(media_info_, ec);
            if (ec)
                return ec;
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
                add_stream(stream);
                streams_.push_back(stream);
                for(boost::uint32_t j = 0; j < transfers_[i].size(); ++j) {
                    transfers_[i][j]->transfer(stream);
                }
            }
            if (!ec) {
                filters_.last()->open(media_info_, streams_, ec);
            }
            return ec;
        }

        error_code MuxerBase::read(
            Sample & tag,
            error_code & ec)
        {
            ec.clear();
            if (!is_open()) {
                ec = error::mux_not_open;
                return ec;
            }
            if (paused_) {
                paused_ = false;
            }

            if (0 == read_step_) {
                read_step_ = 1;
                tag.itrack = boost::uint32_t(-1);
                tag.idesc = boost::uint32_t(-1);
                tag.flags = 0;
                tag.ustime = 0;
                tag.context = NULL;
                file_header(tag);
                if (!tag.data.empty())
                    return ec;
            } else if (read_step_ > 0 && read_step_ != boost::uint32_t(-1)) {
                while(read_step_ <= transfers_.size()) {
                    stream_header(read_step_-1, tag);
                    read_step_++;
                    if (!tag.data.empty()) 
                        return ec;
                }
                read_step_ = boost::uint32_t(-1);
            }
            MuxerBase::get_sample_with_transfer(tag, ec);
            return ec;
        }

        void MuxerBase::reset(void)
        {
            read_step_ = 0;
        }

        error_code MuxerBase::time_seek(
            boost::uint64_t & time,
            error_code & ec)
        {
            if (!is_open()) {
                ec = error::mux_not_open;
            } else {
                demuxer_->seek(time, ec);
                if (!ec || ec == boost::asio::error::would_block) {
                    play_time_ = time;
                    if (filters_.size() < 2) {
                        filters_.push_back(&key_filter_);
                    }
                    for (boost::uint32_t i = 0; i < transfers_.size(); ++i) {
                        for (boost::uint32_t j = 0; j < transfers_[i].size(); ++j) {
                            transfers_[i][j]->on_seek(time);
                        }
                    }
                }
            }
            return ec;
        }

        error_code MuxerBase::byte_seek(
            boost::uint64_t & offset,
            boost::system::error_code & ec)
        {
            boost::uint64_t seek_time = (offset * media_info_.duration) / media_info_.file_size;
            return time_seek(seek_time, ec);
        }

        void MuxerBase::close(void)
        {
            demuxer_ = NULL;
            release_info();
        }

        void MuxerBase::media_info(
            MediaInfo & info) const
        {
            boost::system::error_code ec;
            demuxer_->media().get_info(info, ec);
            info.file_size = ppbox::data::invalid_size;
            info.format = format_;
        }

        error_code MuxerBase::get_sample(
            Sample & sample,
            error_code & ec)
        {
            if (!is_open()) {
                ec = error::mux_not_open;
            } else {
                paused_ = false;
                filters_.last()->get_sample(sample, ec);
                if (!ec) {
                    sample.media_info = &streams_[sample.itrack];
                    play_time_ = sample.time;
                }
            }
            return ec;
        }

        error_code MuxerBase::get_sample_with_transfer(
            Sample & sample,
            error_code & ec)
        {
            get_sample(sample, ec);
            if (!ec) {
                std::vector<Transfer *> & transfers = transfers_[sample.itrack];
                //if (sample.flags & Sample::stream_changed) {
                    //release_info();
                    //open_impl(ec);
                //}
                for(boost::uint32_t i = 0; i < transfers.size(); ++i) {
                    transfers[i]->transfer(sample);
                }
            }
            return ec;
        }

        void MuxerBase::release_info(void)
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
