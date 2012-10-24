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
            std::string const & name, 
            register_type func)
        {
            muxer_map().insert(std::make_pair(name, func));
            return;
        }

        MuxerBase * MuxerBase::create(
            std::string const & proto)
        {
            std::map< std::string, register_type >::iterator iter = muxer_map().find(proto);
            if (muxer_map().end() == iter) {
                return NULL;
            }
            return iter->second();
        }

        void MuxerBase::destory(
            MuxerBase* & source)
        {
            delete source;
            source = NULL;
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
            for (size_t i = 0; i < stream_count; ++i) {
                StreamInfo stream;
                demuxer_->get_stream_info(i, stream, ec);
                if (ec) {
                    break;
                }
                stream.attachment = NULL;
                // TODO: add attachment
                if (stream.type == MEDIA_TYPE_VIDE 
                    && stream.sub_type == VIDEO_TYPE_AVC1) {
                        stream.codec = new AvcCodec(stream.format_data);
                }
                add_stream(stream);
                for(boost::uint32_t i = 0; i < stream.transfers.size(); ++i) {
                    stream.transfers[i]->transfer(stream);
                }
                media_info_.streams.push_back(stream);
            }
            if (!ec) {
                filters_.last()->open(media_info_, ec);
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
                while(read_step_ <= media_info_.streams.size()) {
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
                    for (boost::uint32_t i = 0; i < media_info_.streams.size(); ++i) {
                        for (boost::uint32_t j = 0; j < media_info_.streams[i].transfers.size(); ++j) {
                            media_info_.streams[i].transfers[j]->on_seek(time);
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
            info = media_info_;
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
                    sample.media_info = &media_info_.streams[sample.itrack];
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
                StreamInfo & info = media_info_.streams[sample.itrack];
                //if (sample.flags & Sample::stream_changed) {
                    //release_info();
                    //open_impl(ec);
                //}
                for(boost::uint32_t i = 0; i < info.transfers.size(); ++i) {
                    info.transfers[i]->transfer(sample);
                }
            }
            return ec;
        }

        void MuxerBase::release_info(void)
        {
            for (boost::uint32_t i = 0; i < media_info_.streams.size(); ++i) {
                if (media_info_.streams[i].codec) {
                    delete media_info_.streams[i].codec;
                }
                for (boost::uint32_t j = 0; j < media_info_.streams[i].transfers.size(); ++j) {
                    delete media_info_.streams[i].transfers[j];
                }
                media_info_.streams[i].transfers.clear();
            }
            media_info_.streams.clear();
        }

    } // namespace mux
} // namespace ppbox
