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
            : state_(closed)
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
                MuxerBase::demuxer() = demuxer;
                MuxerBase::media_info().init();
                boost::uint32_t stream_count = MuxerBase::demuxer()->get_media_count(ec);
                if (!ec) {
                    ppbox::demux::MediaInfo media_info;
                    for (size_t i = 0; i < stream_count; ++i) {
                        std::cout << "Stream: " << i << std::endl;
                        MuxerBase::demuxer()->get_media_info(i, media_info, ec);
                        if (ec) {
                            break;
                        } else {
                            if (media_info.type == MEDIA_TYPE_VIDE) {
                                MuxerBase::media_info().frame_rate = media_info.video_format.frame_rate;
                                MuxerBase::media_info().width      = media_info.video_format.width;
                                MuxerBase::media_info().height     = media_info.video_format.height;
                                MuxerBase::media_info().video_format_type = media_info.format_type;
                                error_code lec;
                                boost::uint32_t video_duration = MuxerBase::demuxer()->get_duration(lec);
                                if (lec == framework::system::logic_error::not_supported) {
                                    MuxerBase::media_info().duration = 0;
                                } else {
                                    MuxerBase::media_info().duration   = video_duration / 1000;
                                }
                                MuxerBase::media_info().video_codec = media_info.sub_type;
                                if (media_info.format_type == MediaInfo::video_avc_byte_stream) { // live
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
                                } else { // vod
                                    avc_config_ = new AvcConfig((boost::uint8_t *)&media_info.format_data.at(0)
                                    , media_info.format_data.size());
                                    if (avc_config_->creat()) {
                                        AP4_Result result = ts_writer_.SetVideoStream(media_info.time_scale,
                                            video_stream_, avc_config_);
                                        if (AP4_FAILED(result)) {
                                            ec = ppbox::demux::error::bad_file_format;
                                        } else {
                                            video_stream_->set_spec_config(media_info.format_data);
                                        }
                                    } else {
                                        ec = ppbox::demux::error::bad_file_format;
                                    }
                                }
                                MuxerBase::video_track_index() = i;
                            } else {
                                MuxerBase::audio_track_index() = i;
                                MuxerBase::media_info().sample_rate = media_info.audio_format.sample_rate;
                                MuxerBase::media_info().sample_size = media_info.audio_format.sample_size;
                                MuxerBase::media_info().channel_count = media_info.audio_format.channel_count;
                                MuxerBase::media_info().audio_codec = media_info.sub_type;
                                MuxerBase::media_info().audio_format_type = media_info.format_type;
                                AP4_Result result;
                                if (media_info.format_type == MediaInfo::audio_microsoft_wave) { // live1
                                    result = ts_writer_.SetAudioStream(1000,
                                        audio_stream_);
                                } else { // vod
                                    result = ts_writer_.SetAudioStream(media_info.time_scale,
                                        audio_stream_);
                                }
                                if (AP4_FAILED(result)) {
                                    std::cout << "could not create audio stream, " << result << std::endl;
                                    ec = ppbox::demux::error::bad_file_format;
                                } else {
                                    audio_stream_->set_spec_config(media_info.format_data);
                                }
                            }
                        }
                    } // End for
                }
            }
            if (!ec) {
                if (video_stream_ && audio_stream_) {
                    state_ = opened;
                    ts_writer_.WritePAT(*pmt_);
                    ts_writer_.WritePMT(*pmt_);
                } else {
                    ec = ppbox::mux::error::mux_other_error;
                }
            } else {
                state_ = closed;
            }
            return ec;
        }

        error_code TsMux::read(
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
                get_sort_sample(sample_, ec);
                //MuxerBase::demuxer()->get_sample(sample_, ec);
                if (!ec) {
                    tag->idesc   = sample_.idesc;
                    tag->is_sync = sample_.is_sync;
                    tag->itrack  = sample_.itrack;
                    tag->time    = sample_.time;
                    AP4_Position p;
                    if (sample_.itrack == MuxerBase::video_track_index()) {
                        MuxerBase::current_time() = sample_.time;
                        ts_sample_->Seek(0);
                        video_stream_->WriteSample(sample_, MuxerBase::media_info(), true, *ts_sample_);
                        tag->tag_header_length = 0;
                        tag->tag_header_buffer = NULL;
                        ts_sample_->Tell(p);
                        tag->tag_data_length   = p;
                        tag->tag_data_buffer   = ts_sample_->GetData();
                        tag->tag_size_length   = 0;
                        tag->tag_size_buffer   = NULL;
                    } else if (sample_.itrack == MuxerBase::audio_track_index()) {
                        ts_sample_->Seek(0);
                        audio_stream_->WriteSample(sample_, MuxerBase::media_info(), false, *ts_sample_);
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
                //MuxerBase::demuxer()->get_sample_buffered(sampleTemp, ec);
                get_ignored_sample(sampleTemp, ec);

                if (ec) {
                    return ec;
                }

                if (MuxerBase::video_track_index() == sampleTemp.itrack) {
                    queue_sample_[0].push_back(sampleTemp);
                } else if (MuxerBase::audio_track_index() == sampleTemp.itrack) {
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
                    //MuxerBase::demuxer()->get_sample(sample, ec);
                    MuxerBase::demuxer()->get_sample_buffered(sample, ec);
                    if (ec)
                        break;
                    if (sample.itrack == MuxerBase::video_track_index() && sample.is_sync) {
                        is_wait_sync_ = false;
                        break;
                    }
                }
            } else {
                //MuxerBase::demuxer()->get_sample(sample, ec);
                MuxerBase::demuxer()->get_sample_buffered(sample, ec);
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
                MuxerBase::demuxer()->seek(time, ec);
                if (!ec || ec == boost::asio::error::would_block) {
                    MuxerBase::current_time() = time / 1000;
                }
            }
            return ec;
        }

        error_code TsMux::pause(error_code & ec)
        {
            if (state_ != opened) {
                ec = error::mux_not_open;
            } else {
                MuxerBase::demuxer()->pause(ec);
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
                MuxerBase::demuxer()->resume(ec);
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
