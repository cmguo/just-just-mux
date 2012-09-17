// Muxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/rtp/RtpPacket.h"

#include <ppbox/demux/base/BufferDemuxer.h>
#include <ppbox/demux/base/SourceError.h>

#include <ppbox/data/MediaBase.h>

#include <ppbox/avformat/asf/AsfObjectType.h>
#include <ppbox/avformat/codec/AvcCodec.h>
using namespace ppbox::avformat;
using namespace ppbox::demux;

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
            demux::BufferDemuxer * demuxer,
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

        error_code MuxerBase::open_impl(error_code & ec)
        {
            assert(demuxer_ != NULL);
            demuxer_->get_media_info(media_info_.duration_info, ec);
            media_info_.stream_count = demuxer_->get_stream_count(ec);
            if (!ec) {
                media_info_.filesize = 0;
                for (size_t i = 0; i < media_info_.stream_count; ++i) {
                    MediaInfoEx media_info;
                    demuxer_->get_stream_info(i, media_info, ec);
                    if (ec) {
                        break;
                    } else {
                        media_info.attachment = NULL;
                        // add attachment
                        if (media_info.type == ppbox::demux::MEDIA_TYPE_VIDE 
                            && media_info.sub_type == ppbox::demux::VIDEO_TYPE_AVC1) {
                                media_info.decode = new AvcCodec();
                                media_info.decode->config(
                                    media_info.format_data, media_info.config);
                        }
                        add_stream(media_info);
                        for(boost::uint32_t i = 0; i < media_info.transfers.size(); ++i) {
                            media_info.transfers[i]->transfer(media_info);
                        }
                        media_info_.stream_infos.push_back(media_info);
                    }
                }
            }
            if (!ec) {
                filters_.last()->open(media_info_, ec);
            }
            return ec;
        }

        error_code MuxerBase::read(
            ppbox::demux::Sample & tag,
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
                while(read_step_ <= media_info_.stream_count) {
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

        error_code MuxerBase::seek(
            boost::uint32_t & time,
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
                    for (boost::uint32_t i = 0; i < media_info_.stream_infos.size(); ++i) {
                        for (boost::uint32_t j = 0; j < media_info_.stream_infos[i].transfers.size(); ++j) {
                            media_info_.stream_infos[i].transfers[j]->on_seek(time);
                        }
                    }
                }
            }
            return ec;
        }

        error_code MuxerBase::byte_seek(
            boost::uint32_t & offset,
            boost::system::error_code & ec)
        {
            boost::uint32_t seek_time = (offset * media_info_.duration_info.duration) / media_info_.filesize;
            return seek(seek_time, ec);
        }

        error_code MuxerBase::get_duration(
            ppbox::data::MediaInfo & info, 
            error_code & ec)
        {
            if (!is_open()) {
                ec = error::mux_not_open;
            } else {
                demuxer_->get_media_info(info, ec);
            }
            return ec;
        }

        error_code MuxerBase::pause(
            error_code & ec)
        {
            if (!is_open()) {
                ec = error::mux_not_open;
            } else {
                demuxer_->pause(ec);
                if (!ec) {
                    paused_ = true;
                }
            }
            return ec;
        }

        error_code MuxerBase::resume(
            boost::system::error_code & ec)
        {
            if (!is_open()) {
                ec = error::mux_not_open;
            } else {
                demuxer_->resume(ec);
                if (!ec) {
                    paused_ = false;
                }
            }
            return ec;
        }

        void MuxerBase::close(void)
        {
            demuxer_ = NULL;
            release_mediainfo();
        }

        boost::uint32_t & MuxerBase::current_time(void)
        {
            return play_time_;
        }

        MediaFileInfo & MuxerBase::mediainfo(void)
        {
            return media_info_;
        }

        framework::configure::Config & MuxerBase::config()
        {
            return config_;
        }

        error_code MuxerBase::get_buffer_time(
            boost::uint32_t & buffer_time,
            error_code & ec)
        {
            boost::uint32_t cur = demuxer_->get_cur_time(ec);
            if (!ec) {
                error_code ec_buf;
                boost::uint32_t end = demuxer_->get_end_time(ec, ec_buf);
                if (!ec) {
                    buffer_time = end > cur ? (end-cur) : 0;
                    if (ec_buf == source_error::no_more_segment) {
                        ec = ec_buf;
                    }
                }
            }
            return ec;
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
                    sample.media_info = &media_info_.stream_infos[sample.itrack];
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
                MediaInfoEx & mediainfo = media_info_.stream_infos[sample.itrack];
                //if (sample.flags & Sample::stream_changed) {
                    //release_mediainfo();
                    //open_impl(ec);
                //}
                for(boost::uint32_t i = 0; i < mediainfo.transfers.size(); ++i) {
                    mediainfo.transfers[i]->transfer(sample);
                }
            }
            return ec;
        }

        void MuxerBase::release_mediainfo(void)
        {
            for (boost::uint32_t i = 0; i < media_info_.stream_infos.size(); ++i) {
                if (media_info_.stream_infos[i].decode) {
                    delete media_info_.stream_infos[i].decode;
                    media_info_.stream_infos[i].decode = NULL;
                }
                for (boost::uint32_t j = 0; j < media_info_.stream_infos[i].transfers.size(); ++j) {
                    delete media_info_.stream_infos[i].transfers[j];
                }
                media_info_.stream_infos[i].transfers.clear();
            }
            media_info_.stream_infos.clear();
        }

    } // namespace mux
} // namespace ppbox
