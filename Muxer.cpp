// Muxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/tinyvlc.h"
#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/H264Nalu.h"
#include "ppbox/mux/decode/AvcDecode.h"

#include "ppbox/mux/rtp/RtpPacket.h"
#include <ppbox/demux/base/BufferDemuxer.h>

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
                MediaInfoEx media_info;
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
                            // add attachment
                            if (media_info.type == ppbox::demux::MEDIA_TYPE_VIDE 
                                && media_info.sub_type == ppbox::demux::VIDEO_TYPE_AVC1) {
                                media_info_.stream_infos[i].decode = new AvcDecode();
                                media_info_.stream_infos[i].decode->config(
                                    media_info.format_data, media_info_.stream_infos[i].config);
                            }

                            std::vector<Transfer *> transfers;
                            add_stream(media_info_.stream_infos[i], transfers);
                            stream_transfers_.push_back(transfers);
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
            release_decode();
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
                std::vector<Transfer *> & stream_transfer = stream_transfers_[sample.itrack];
                //if (sample_.flags & Sample::stream_changed) {
                MediaInfoEx & mediainfo = media_info_.stream_infos[sample.itrack];
                for(boost::uint32_t i = 0; i < stream_transfer.size(); ++i) {
                    stream_transfer[i]->transfer(mediainfo);
                }
                //}

                for(boost::uint32_t i = 0; i < stream_transfer.size(); ++i) {
                    stream_transfer[i]->transfer(sample);
                }
            }
            return ec;
        }

        error_code Muxer::mediainfo_translater(
            MediaInfoEx & stream_info,
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

        void Muxer::release_transfer(void)
        {
            for (boost::uint32_t i = 0; i < stream_transfers_.size(); ++i) {
                std::vector<Transfer *> & stream_transfer = stream_transfers_[i];
                for(boost::uint32_t j = 0; j < stream_transfer.size(); ++j) {
                    delete stream_transfer[j];
                }
                stream_transfer.clear();
            }
            stream_transfers_.clear();
        }

        void Muxer::release_decode(void)
        {
            for (boost::uint32_t i = 0; i < media_info_.stream_infos.size(); ++i) {
                if (media_info_.stream_infos[i].decode) {
                    delete media_info_.stream_infos[i].decode;
                    media_info_.stream_infos[i].decode = NULL;
                }
            }
        }
    }
}
