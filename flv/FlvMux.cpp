// FlvMux.cpp
#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvMux.h"
#include "ppbox/mux/flv/FlvMuxWriter.h"

#include <ppbox/demux/Demuxer.h>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {
        FlvMux::FlvMux()
            : state_(closed)
            , flv_writer_(new FlvMuxWriter())
            , paused_(false)
            , is_read_head_(false)
            , is_read_metadata_(false)
        {
        }

        FlvMux::~FlvMux()
        {
            if (flv_writer_) {
                delete flv_writer_;
                flv_writer_ = NULL;
            }
        }

        error_code FlvMux::open(
            demux::Demuxer * demuxer, error_code & ec)
        {
            assert(demuxer != NULL);
            ec.clear();
            if (state_ != closed) {
                ec = error::mux_already_open;
            } else {
                MuxerBase::demuxer() = demuxer;
                //get stream
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
                            flv_writer_->open(i, media_info);
                            if (media_info.type == MEDIA_TYPE_VIDE) {
                                MuxerBase::media_info().frame_rate = media_info.video_format.frame_rate;
                                MuxerBase::media_info().width      = media_info.video_format.width;
                                MuxerBase::media_info().height     = media_info.video_format.height;
                                MuxerBase::media_info().video_format_type = media_info.format_type;
                                error_code lec;
                                boost::uint32_t video_duration = MuxerBase::demuxer()->get_duration(lec);
                                if (lec) {
                                    MuxerBase::media_info().duration = 0;
                                } else {
                                    MuxerBase::media_info().duration   = video_duration / 1000;
                                }
                                MuxerBase::media_info().video_codec = media_info.sub_type;
                                MuxerBase::video_track_index() = i;
                            } else {
                                MuxerBase::audio_track_index() = i;
                                // 声道数为2，live 和 vod都一样
                                MuxerBase::media_info().sample_rate = media_info.audio_format.sample_rate;
                                MuxerBase::media_info().sample_size = media_info.audio_format.sample_size;
                                MuxerBase::media_info().channel_count = media_info.audio_format.channel_count;
                                MuxerBase::media_info().audio_format_type = media_info.format_type;
                                MuxerBase::media_info().audio_codec = media_info.sub_type;
                            }
                        }
                    } // End for
                }
            }
            if (!ec) {
                state_ = opened;
                flv_writer_->set_media_info(&MuxerBase::media_info());
                //create_metadata();
            } else {
                state_ = closed;
            }
            return ec;
        }

        error_code FlvMux::read(
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
                MuxerBase::demuxer()->get_sample_buffered(sample_, ec);
                if (!ec) {
                    tag->idesc   = sample_.idesc;
                    tag->is_sync = sample_.is_sync;
                    tag->itrack  = sample_.itrack;
                    tag->time    = sample_.time;
                    if (sample_.itrack == MuxerBase::video_track_index()) {
                        MuxerBase::current_time() = sample_.time;
                        flv_writer_->HandeVideo(sample_);
                        tag->tag_header_length = flv_writer_->get_tag()->tag_header_length;
                        tag->tag_header_buffer = flv_writer_->get_tag()->tag_header_buffer;
                        tag->tag_data_length   = flv_writer_->get_tag()->tag_data_length;
                        tag->tag_data_buffer   = flv_writer_->get_tag()->tag_data_buffer;
                        tag->tag_size_length   = flv_writer_->get_tag()->tag_size_length;
                        tag->tag_size_buffer   = flv_writer_->get_tag()->tag_size_buffer;
                    } else if (sample_.itrack == MuxerBase::audio_track_index()) {
                        flv_writer_->HandeAudio(sample_);
                        tag->tag_header_length = flv_writer_->get_tag()->tag_header_length;
                        tag->tag_header_buffer = flv_writer_->get_tag()->tag_header_buffer;
                        tag->tag_data_length   = flv_writer_->get_tag()->tag_data_length;
                        tag->tag_data_buffer   = flv_writer_->get_tag()->tag_data_buffer;
                        tag->tag_size_length   = flv_writer_->get_tag()->tag_size_length;
                        tag->tag_size_buffer   = flv_writer_->get_tag()->tag_size_buffer;
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
                if (!is_read_metadata_) {
                    tag->tag_header_length = 0;
                    tag->tag_header_buffer = NULL;
                    tag->tag_data_length = flv_writer_->get_metadata_size();
                    tag->tag_data_buffer = flv_writer_->get_metadata_buffer();
                    tag->tag_size_length = 0;
                    tag->tag_size_buffer = NULL;
                    is_read_metadata_ = true;
                } else {
                    tag->tag_header_length = 0;
                    tag->tag_header_buffer = NULL;
                    tag->tag_data_length = flv_writer_->get_header_size();
                    tag->tag_data_buffer = flv_writer_->get_header_buffer();
                    tag->tag_size_length = 0;
                    tag->tag_size_buffer = NULL;
                    is_read_head_ = true;
                }
            }
            return ec;
        }

        error_code FlvMux::seek(
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

        error_code FlvMux::pause(error_code & ec)
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

        error_code FlvMux::resume(
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

        void FlvMux::close(void)
        {
            if (state_ != closed) {
                state_ = closed;
            }
        }

        void FlvMux::reset(void)
        {
            is_read_head_ = false;
            is_read_metadata_ = false;
        }

        ppbox::demux::Sample & FlvMux::get_sample(void)
        {
            return sample_;
        }

        unsigned char const * FlvMux::get_head(boost::uint32_t &size)
        {
            if (state_ != opened) {
                size = 0;
                return NULL;
            } else {
                size = flv_writer_->get_header_size();
                return flv_writer_->get_header_buffer();
            }
        }

    } // namespace mux
} // namespace ppbox
