// Muxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/tinyvlc.h"
#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/rtp/RtpPacket.h"
#include <ppbox/demux/base/BufferDemuxer.h>

#include <ppbox/avformat/asf/AsfObjectType.h>
#include <ppbox/avformat/codec/AvcCodec.h>
using namespace ppbox::avformat;
using namespace ppbox::demux;
using namespace boost::system;

#include <framework/memory/MemoryPage.h>
#include <util/buffers/BufferCopy.h>
#include <util/archive/ArchiveBuffer.h>
#include <iostream>

namespace ppbox
{
    namespace mux
    {
        error_code Muxer::open(
            demux::BufferDemuxer * demuxer,
            error_code & ec)
        {
            assert(demuxer != NULL);
            demuxer_ = demuxer;
            demux_filter_.set_demuxer(demuxer_);
            media_info_.stream_count = demuxer_->get_media_count(ec);
            if (!ec) {
                error_code lec;
                boost::uint32_t video_duration = demuxer_->get_duration(lec);
                if (lec) {
                    media_info_.duration = 0;
                } else {
                    media_info_.duration   = video_duration;
                }
                media_info_.filesize = media_info_.duration * 1000;
                MediaInfoEx media_info;
                for (size_t i = 0; i < media_info_.stream_count; ++i) {
                    demuxer_->get_media_info(i, media_info, ec);
                    if (ec) {
                        break;
                    } else {
                        media_info.attachment = NULL;
                        media_info_.stream_infos.push_back(media_info);
                        // add attachment
                        if (media_info.type == ppbox::demux::MEDIA_TYPE_VIDE 
                            && media_info.sub_type == ppbox::demux::VIDEO_TYPE_AVC1) {
                            media_info_.stream_infos[i].decode = new AvcCodec();
                            media_info_.stream_infos[i].decode->config(
                                media_info.format_data, media_info_.stream_infos[i].config);
                        }
                        add_stream(media_info_.stream_infos[i]);
                    }
                }
            }
            if (ec) {
                demuxer_ = NULL;
            }
            return ec;
        }

        bool Muxer::is_open()
        {
            if (demuxer_) {
                return true;
            } else {
                return false;
            }
        }

        error_code Muxer::read(
            ppbox::demux::Sample & tag,
            error_code & ec)
        {
            ec.clear();
            if (!is_open()) {
                return error::mux_not_open;
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
            Muxer::get_sample_with_transfer(tag, ec);
            return ec;
        }

        void Muxer::reset(void)
        {
            read_step_ = 0;
        }

        error_code Muxer::seek(
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
                }
            }
            return ec;
        }

        error_code Muxer::byte_seek(
            boost::uint32_t & offset,
            boost::system::error_code & ec)
        {
            boost::uint32_t seek_time = (offset * media_info_.duration) / media_info_.filesize;
            return seek(seek_time, ec);
        }

        error_code Muxer::pause(
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

        error_code Muxer::resume(
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

        void Muxer::close(void)
        {
            demuxer_ = NULL;
            release_mediainfo();
        }

        boost::uint32_t & Muxer::current_time(void)
        {
            return play_time_;
        }

        MediaFileInfo & Muxer::mediainfo(void)
        {
            return media_info_;
        }

        framework::configure::Config & Muxer::Config()
        {
            return config_;
        }

        error_code Muxer::get_buffer_time(
            boost::uint32_t & buffer_time,
            error_code & ec)
        {
            boost::uint32_t cur = demuxer_->get_cur_time(ec);
            if (!ec) {
                error_code ec_buf;
                boost::uint32_t end = demuxer_->get_end_time(ec, ec_buf);
                if (!ec) {
                    buffer_time = end > cur ? (end-cur) : 0;
                    if (ec_buf == boost::asio::error::eof) {
                        ec = ec_buf;
                    }
                }
            }
            return ec;
        }

        error_code Muxer::get_sample(
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

        error_code Muxer::get_sample_with_transfer(
            Sample & sample,
            error_code & ec)
        {
            get_sample(sample, ec);
            if (!ec) {
                MediaInfoEx & mediainfo = media_info_.stream_infos[sample.itrack];
                //if (sample_.flags & Sample::stream_changed) {
                for(boost::uint32_t i = 0; i < mediainfo.transfers.size(); ++i) {
                    mediainfo.transfers[i]->transfer(mediainfo);
                }
                //}

                for(boost::uint32_t i = 0; i < mediainfo.transfers.size(); ++i) {
                    mediainfo.transfers[i]->transfer(sample);
                }
            }
            return ec;
        }

        void Muxer::release_mediainfo(void)
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

    }
}
