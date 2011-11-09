// Muxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/tinyvlc.h"
#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/H264Nalu.h"

#include <ppbox/demux/Demuxer.h>
#include <ppbox/demux/asf/AsfObjectType.h>
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
            demux::Demuxer * demuxer,
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
                ppbox::demux::MediaInfo media_info;
                attachment_ = "";
                for (size_t i = 0; i < media_info_.stream_count; ++i) {
                    demuxer_->get_media_info(i, media_info, ec);
                    if (ec) {
                        break;
                    } else {
                        mediainfo_translater(media_info, ec);
                        if (ec)
                            break;
                        else {
                            media_info.attachment = NULL;
                            media_info_.stream_infos.push_back(media_info);
                            media_info_.stream_infos[i].attachment = NULL;
                            if (media_info.type == MEDIA_TYPE_VIDE) {
                                media_info_.video_index = i;
                                add_stream(media_info_.stream_infos[i], video_transfers_);
                            } else {
                                media_info_.audio_index = i;
                                add_stream(media_info_.stream_infos[i], audio_transfers_);
                            }
                        }
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

            if (is_read_head_) {
                Sample const & sample = Muxer::get_sample_with_transfer(ec);
                if (!ec) {
                    tag = sample;
                }
            } else {
                is_read_head_ = true;
                tag.itrack = boost::uint32_t(-1);
                tag.idesc = boost::uint32_t(-1);
                tag.flags = 0;
                tag.ustime = 0;
                tag.context = NULL;
                head_buffer(tag);
            }
            return ec;
        }

        void Muxer::reset(void)
        {
            is_read_head_ = false;
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
                }
            }
            return ec;
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
            release_transfer();
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

        Sample & Muxer::get_sample(
            error_code & ec)
        {
            if (!is_open()) {
                ec = error::mux_not_open;
            } else {
                paused_ = false;
                filters_.last()->get_sample(sample_, ec);
                if (!ec) {
                    sample_.media_info = &media_info_.stream_infos[sample_.itrack];
                    play_time_ = sample_.time;
                }
            }
            return sample_;
        }

        Sample & Muxer::get_sample_with_transfer(
            error_code & ec)
        {
            get_sample(ec);
            if (!ec) {
                transfer_sample(sample_);
            }
            return sample_;
        }

        error_code Muxer::mediainfo_translater(
            MediaInfo & stream_info,
            error_code & ec)
        {
            if (stream_info.sub_type == VIDEO_TYPE_AVC1 
                && stream_info.format_type == MediaInfo::video_avc_byte_stream) {
                    Buffer_Array config_list;
                    H264Nalu::process_live_video_config(
                        &stream_info.format_data.at(0),
                        stream_info.format_data.size(),
                        config_list);
                    AvcConfig avc_config((boost::uint32_t)framework::memory::MemoryPage::align_page(stream_info.format_data.size() * 2));
                    if (config_list.size() >= 2) {
                        Buffer_Array spss;
                        Buffer_Array ppss;
                        spss.push_back(config_list[config_list.size()-2]);
                        ppss.push_back(config_list[config_list.size()-1]);
                        avc_config.creat(0x01, 0x64, 0x00, 0x15, 0x04, spss, ppss);
                        stream_info.format_data.resize(avc_config.data_size());
                        memcpy(&stream_info.format_data.at(0), avc_config.data(), avc_config.data_size());
                    } else {
                        ec = error::mux_invalid_sample;
                    }
            } else if (stream_info.format_type == MediaInfo::audio_microsoft_wave) {
                boost::uint8_t cbuf[1024];
                memset(cbuf, 0, sizeof(cbuf));
                memcpy(cbuf, &stream_info.format_data.at(0), stream_info.format_data.size());
                util::archive::ArchiveBuffer<boost::uint8_t> buf(
                    cbuf,
                    sizeof(cbuf), 
                    stream_info.format_data.size());
                ASF_Audio_Media_Type asf_audio_type;
                ASFArchive archive(buf);
                archive >> asf_audio_type;
                if (asf_audio_type.CodecSpecificDataSize == 0) {
                    stream_info.format_data.clear();
                } else {
                    stream_info.format_data.assign(
                        asf_audio_type.CodecSpecificData.begin(), 
                        asf_audio_type.CodecSpecificData.end());
                }
            }
            return ec;
        }

        void Muxer::transfer_sample(Sample & sample)
        {
            if(sample.itrack == media_info_.video_index) {
                for(boost::uint32_t i = 0; i < video_transfers_.size(); ++i) {
                    video_transfers_[i]->transfer(sample);
                }
            } else if (sample.itrack == media_info_.audio_index) {
                for(boost::uint32_t i = 0; i < audio_transfers_.size(); ++i) {
                    audio_transfers_[i]->transfer(sample);
                }
            }
        }

        void Muxer::release_transfer(void)
        {
            for(boost::uint32_t i = 0; i < video_transfers_.size(); ++i) {
                delete video_transfers_[i];
            }
            video_transfers_.clear();
            for(boost::uint32_t i = 0; i < audio_transfers_.size(); ++i) {
                delete audio_transfers_[i];
            }
            audio_transfers_.clear();
        }
    }
}
