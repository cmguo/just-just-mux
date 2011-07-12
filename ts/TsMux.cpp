// TsMux.cpp
#include "ppbox/mux/Common.h"
#include "ppbox/mux/H264Nalu.h"
#include "ppbox/mux/ts/TsMux.h"
#include "ppbox/mux/ts/Ap4Mpeg2Ts.h"

#include <framework/memory/MemoryPage.h>

#include <ppbox/demux/Demuxer.h>
#include <ppbox/demux/DemuxerError.h>
using namespace ppbox::demux;

#include <iostream>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {
        TsMux::TsMux()
            : demuxer_(NULL)
            , state_(closed)
            , current_tag_time_(0)
            , paused_(false)
            , is_read_head_(false)
            , ipad_(false)
            , is_wait_sync_(false)
            , pmt_(new AP4_MemoryByteStream(512))
            , ts_sample_(new AP4_MemoryByteStream(1024 * 1024))
            , audio_stream_(NULL)
            , video_stream_(NULL)
            , avc_config_(NULL)
        {
        }

        TsMux::~TsMux()
        {
            if (pmt_) {
                pmt_->Release();
            }

            if (ts_sample_) {
                ts_sample_->Release();
            }

            if (avc_config_) {
                delete avc_config_;
                avc_config_ = NULL;
            }
        }

        error_code TsMux::open(
            demux::Demuxer * demuxer, error_code & ec)
        {
            assert(demuxer != NULL);
            ec.clear();
            if (state_ != closed) {
                ec = error::mux_already_open;
            } else {
                demuxer_ = demuxer;
                media_file_info_.init();
                boost::uint32_t stream_count = demuxer_->get_media_count(ec);
                if (!ec) {
                    ppbox::demux::MediaInfo media_info;
                    for (size_t i = 0; i < stream_count; ++i) {
                        std::cout << "Stream: " << i << std::endl;
                        demuxer_->get_media_info(i, media_info, ec);
                        if (ec) {
                            break;
                        } else {
                            if (media_info.type == MEDIA_TYPE_VIDE) {
                                media_file_info_.frame_rate = media_info.video_format.frame_rate;
                                media_file_info_.width      = media_info.video_format.width;
                                media_file_info_.height     = media_info.video_format.height;
                                media_file_info_.video_format_type = media_info.format_type;
                                error_code lec;
                                boost::uint32_t video_duration = demuxer_->get_duration(lec);
                                if (lec == framework::system::logic_error::not_supported) {
                                    media_file_info_.duration = 0;
                                } else {
                                    media_file_info_.duration   = video_duration / 1000;
                                }
                                media_file_info_.video_codec = media_info.sub_type;
                                if (media_info.format_type == MediaInfo::video_avc_byte_stream) {
                                    // live
                                    Buffer_Array config_list;
                                    H264Nalu::process_live_video_config(
                                        (boost::uint8_t *)&media_info.format_data.at(0),
                                        media_info.format_data.size(),
                                        config_list);
                                    if (config_list.size() >= 2) {
                                        // 分配足够保存video avc config信息
                                        avc_config_ = new AvcConfig();
                                        if (config_list.size() >= 2) {
                                            Buffer_Array spss;
                                            Buffer_Array ppss;
                                            spss.push_back(config_list[config_list.size()-2]);
                                            ppss.push_back(config_list[config_list.size()-1]);
                                            // 设置avc config
                                            avc_config_->set(0x01, 0x64, 0x00, 0x15, 0x04, spss, ppss);
                                            AP4_Result result = ts_writer_.SetVideoStream(1000,
                                                video_stream_, avc_config_);
                                            if (AP4_FAILED(result)) {
                                                ec = ppbox::demux::error::bad_file_format;
                                            }
                                        }
                                    } else {
                                        ec = ppbox::demux::error::bad_file_format;
                                    }
                                } else {
                                    // vod
                                    avc_config_ = new AvcConfig((boost::uint8_t *)&media_info.format_data.at(0)
                                    , media_info.format_data.size());
                                    if (avc_config_->creat()) {
                                        AP4_Result result = ts_writer_.SetVideoStream(media_info.time_scale,
                                            video_stream_, avc_config_);
                                        if (AP4_FAILED(result)) {
                                            ec = ppbox::demux::error::bad_file_format;
                                        }
                                    } else {
                                        ec = ppbox::demux::error::bad_file_format;
                                    }
                                }
                                video_index_ = i;
                            } else {
                                audio_index_ = i;
                                media_file_info_.sample_rate = media_info.audio_format.sample_rate;
                                media_file_info_.sample_size = media_info.audio_format.sample_size;
                                media_file_info_.channel_count = media_info.audio_format.channel_count;
                                media_file_info_.audio_codec = media_info.sub_type;
                                media_file_info_.audio_format_type = media_info.format_type;

                                AP4_Result result;
                                if (media_info.format_type == MediaInfo::audio_microsoft_wave) {
                                    // live
                                    result = ts_writer_.SetAudioStream(1000,
                                        audio_stream_);
                                } else {
                                    // vod
                                    result = ts_writer_.SetAudioStream(media_info.time_scale,
                                        audio_stream_);
                                }

                                if (AP4_FAILED(result)) {
                                    std::cout << "could not create audio stream, " << result << std::endl;
                                }
                            }
                        }
                    } // End for
                }
            }
            if (!ec) {
                state_ = opened;
                ts_writer_.WritePAT(*pmt_);
                ts_writer_.WritePMT(*pmt_);
            } else {
                state_ = closed;
            }
            return ec;
        }

        error_code TsMux::read(
            MuxTag * tag,
            error_code & ec)
        {
            ec.clear();
            assert(tag != NULL);
            if (state_ != opened) {
                return error::mux_not_open;
            }
            if (paused_) {
                paused_ = false;
            }
            if (is_read_head_) {
                get_ignored_sample(sample_, ec);
                if (!ec) {
                    AP4_Position p;
                    if (sample_.itrack == video_index_) {
                        current_tag_time_ = sample_.time;
                        ts_sample_->Seek(0);
                        video_stream_->WriteSample(sample_, media_file_info_, true, *ts_sample_);
                        tag->tag_header_length = 0;
                        tag->tag_header_buffer = NULL;
                        ts_sample_->Tell(p);
                        tag->tag_data_length   = p;
                        tag->tag_data_buffer   = ts_sample_->GetData();
                        tag->tag_size_length   = 0;
                        tag->tag_size_buffer   = NULL;
                    } else if (sample_.itrack == audio_index_) {
                        ts_sample_->Seek(0);
                        audio_stream_->WriteSample(sample_, media_file_info_, false, *ts_sample_);
                        tag->tag_header_length = 0;
                        tag->tag_header_buffer = NULL;
                        ts_sample_->Tell(p);
                        tag->tag_data_length   = p;
                        tag->tag_data_buffer   = ts_sample_->GetData();
                        tag->tag_size_length   = 0;
                        tag->tag_size_buffer   = NULL;
                    } else {
                        tag->tag_header_length = 0;
                        tag->tag_header_buffer = NULL;
                        tag->tag_data_length   = 0;
                        tag->tag_data_buffer   = NULL;
                        tag->tag_size_length   = 0;
                        tag->tag_size_buffer   = NULL;
                    }
                }
            } else {
                    tag->tag_header_length = 0;
                    tag->tag_header_buffer = NULL;
                    tag->tag_data_length = pmt_->GetDataSize();
                    tag->tag_data_buffer = pmt_->GetData();
                    tag->tag_size_length = 0;
                    tag->tag_size_buffer = NULL;
                    is_read_head_ = true;
            }
            return ec;
        }

        error_code TsMux::readex(
            MuxTagEx * tag,
            error_code & ec)
        {
            ec.clear();
            assert(tag != NULL);
            if (state_ != opened) {
                return error::mux_not_open;
            }
            if (paused_) {
                paused_ = false;
            }
            if (is_read_head_) {
                get_ignored_sample(sample_, ec);
                if (!ec) {
                    tag->idesc   = sample_.idesc;
                    tag->is_sync = sample_.is_sync;
                    tag->itrack  = sample_.itrack;
                    tag->time    = sample_.time;
                    AP4_Position p;
                    if (sample_.itrack == video_index_) {
                        current_tag_time_ = sample_.time;
                        ts_sample_->Seek(0);
                        video_stream_->WriteSample(sample_, media_file_info_, true, *ts_sample_);
                        tag->tag_header_length = 0;
                        tag->tag_header_buffer = NULL;
                        ts_sample_->Tell(p);
                        tag->tag_data_length   = p;
                        tag->tag_data_buffer   = ts_sample_->GetData();
                        tag->tag_size_length   = 0;
                        tag->tag_size_buffer   = NULL;
                    } else if (sample_.itrack == audio_index_) {
                        ts_sample_->Seek(0);
                        audio_stream_->WriteSample(sample_, media_file_info_, false, *ts_sample_);
                        tag->tag_header_length = 0;
                        tag->tag_header_buffer = NULL;
                        ts_sample_->Tell(p);
                        tag->tag_data_length   = p;
                        tag->tag_data_buffer   = ts_sample_->GetData();
                        tag->tag_size_length   = 0;
                        tag->tag_size_buffer   = NULL;
                    } else {
                        tag->tag_header_length = 0;
                        tag->tag_header_buffer = NULL;
                        tag->tag_data_length   = 0;
                        tag->tag_data_buffer   = NULL;
                        tag->tag_size_length   = 0;
                        tag->tag_size_buffer   = NULL;
                    }
                }
            } else {
                tag->idesc   = boost::uint32_t(-1);
                tag->is_sync = false;
                tag->itrack  = boost::uint32_t(-1);
                tag->time    = 0;
                tag->tag_header_length = 0;
                tag->tag_header_buffer = NULL;
                tag->tag_data_length = pmt_->GetDataSize();
                tag->tag_data_buffer = pmt_->GetData();
                tag->tag_size_length = 0;
                tag->tag_size_buffer = NULL;
                is_read_head_ = true;
            }
            return ec;
        }

        error_code TsMux::get_sort_sample(
            ppbox::demux::Sample & sample,
            error_code & ec)
        {
            while (true)
            {
                ec.clear();
                if (queue_sample_[0].size() > 0 && queue_sample_[1].size() > 0) {
                    if (queue_sample_[1].front().time < queue_sample_[0].front().time) {
                        sample =  queue_sample_[1].front();
                        queue_sample_[1].pop_front();
                    } else {
                        sample =  queue_sample_[0].front();
                        queue_sample_[0].pop_front();
                    }
                    return ec;
                }

                Sample sampleTemp;
                demuxer_->get_sample_buffered(sampleTemp, ec);

                if (ec) {
                    return ec;
                }

                if (video_index_ == sampleTemp.itrack) {
                    queue_sample_[0].push_back(sampleTemp);
                } else if (audio_index_ == sampleTemp.itrack) {
                    queue_sample_[1].push_back(sampleTemp);
                }
            }
            return ec;
        }

        error_code TsMux::get_ignored_sample(
            ppbox::demux::Sample& sample,
            boost::system::error_code & ec)
        {
            if (is_wait_sync_) {
                while (true) {
                    demuxer_->get_sample(sample, ec);
                    if (ec)
                        break;
                    if (sample.itrack == video_index_ && sample.is_sync) {
                        is_wait_sync_ = false;
                        break;
                    }
                }
            } else {
                demuxer_->get_sample(sample, ec);
            }
            return ec;
        }

        error_code TsMux::seek(
            boost::uint32_t time,
            error_code & ec)
        {
            if (state_ != opened) {
                ec = error::mux_not_open;
            } else {
                demuxer_->seek(time, ec);
                if (!ec || ec == boost::asio::error::would_block) {
                    current_tag_time_ = time / 1000;
                }
            }
            return ec;
        }

        error_code TsMux::pause(error_code & ec)
        {
            if (state_ != opened) {
                ec = error::mux_not_open;
            } else {
                demuxer_->pause(ec);
                if (!ec) {
                    paused_ = true;
                }
            }
            return ec;
        }

        error_code TsMux::resume(
            error_code & ec)
        {
            if (state_ != opened) {
                ec = error::mux_not_open;
            } else {
                demuxer_->resume(ec);
                if (!ec) {
                    paused_ = false;
                }
            }
            return ec;
        }

        void TsMux::close(void)
        {
            if (state_ != closed) {
                state_ = closed;
            }
        }

        void TsMux::reset(void)
        {
            is_read_head_ = false;
        }

        ppbox::demux::Sample & TsMux::get_sample(void)
        {
            return sample_;
        }

        unsigned char const * TsMux::get_head(boost::uint32_t & size)
        {
            if (state_ != opened) {
                size = 0;
                return NULL;
            } else {
                size = pmt_->GetDataSize();
                return pmt_->GetData();
            }
        }

        MediaFileInfo const & TsMux::get_media_info(void) const
        {
            return media_file_info_;
        }

        boost::uint64_t TsMux::get_current_time(void)
        {
            return current_tag_time_;
        }

        boost::uint32_t TsMux::video_track_index(void)
        {
            return video_index_;
        }

        boost::uint32_t TsMux::audio_track_index(void)
        {
            return audio_index_;
        }

        void TsMux::set_ipad(bool is_ipad)
        {
            ipad_ = is_ipad;
            if (ipad_) {
                is_wait_sync_ = true;
            } else {
                is_wait_sync_ = false;
            }
            if (audio_stream_) {
                audio_stream_->set_ipad_stream(true);
            }
            if (video_stream_) {
                video_stream_->set_ipad_stream(true);
            }
        }

    } // namespace mux
} // namespace ppbox
