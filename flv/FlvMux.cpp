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
            : demuxer_(NULL)
            , state_(closed)
            , flv_writer_(new FlvMuxWriter())
            , current_tag_time_(0)
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
                demuxer_ = demuxer;
                //get stream
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
                            flv_writer_->open(i, media_info);
                            if (media_info.type == MEDIA_TYPE_VIDE) {
                                media_file_info_.frame_rate = media_info.video_format.frame_rate;
                                media_file_info_.width      = media_info.video_format.width;
                                media_file_info_.height     = media_info.video_format.height;
                                media_file_info_.video_format_type = media_info.format_type;
                                error_code lec;
                                boost::uint32_t video_duration = demuxer_->get_duration(lec);
                                if (lec) {
                                    media_file_info_.duration = 0;
                                } else {
                                    media_file_info_.duration   = video_duration / 1000;
                                }
                                media_file_info_.video_codec = media_info.sub_type;
                                video_index_ = i;
                            } else {
                                audio_index_ = i;
                                // 声道数为2，live 和 vod都一样
                                media_info.audio_format.channel_count = media_info.audio_format.channel_count;
                                media_file_info_.sample_rate = media_info.audio_format.sample_rate;
                                media_file_info_.sample_size = media_info.audio_format.sample_size;
                                media_file_info_.channel_count = media_info.audio_format.channel_count;
                                media_file_info_.audio_format_type = media_info.format_type;
                                media_file_info_.audio_codec = media_info.sub_type;
                            }
                        }
                    } // End for
                }
            }
            if (!ec) {
                state_ = opened;
                flv_writer_->set_media_info(&media_file_info_);
                //create_metadata();
            } else {
                state_ = closed;
            }
            return ec;
        }

        error_code FlvMux::read(
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
                //demuxer_->get_sample_buffered(sample_, ec);
                demuxer_->get_sample(sample_, ec);
                if (!ec) {
                    if (sample_.itrack == video_index_) {
                        current_tag_time_ = sample_.time;
                        flv_writer_->HandeVideo(sample_);
                        tag->tag_header_length = flv_writer_->get_tag()->tag_header_length;
                        tag->tag_header_buffer = flv_writer_->get_tag()->tag_header_buffer;
                        tag->tag_data_length   = flv_writer_->get_tag()->tag_data_length;
                        tag->tag_data_buffer   = flv_writer_->get_tag()->tag_data_buffer;
                        tag->tag_size_length   = flv_writer_->get_tag()->tag_size_length;
                        tag->tag_size_buffer   = flv_writer_->get_tag()->tag_size_buffer;
                    } else if (sample_.itrack == audio_index_) {
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
                demuxer_->seek(time, ec);
                if (!ec || ec == boost::asio::error::would_block) {
                    current_tag_time_ = time / 1000;
                }
            }
            return ec;
        }

        error_code FlvMux::pause(error_code & ec)
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

        error_code FlvMux::resume(
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

        MediaFileInfo const & FlvMux::get_media_info(void) const
        {
            return media_file_info_;
        }

        boost::uint64_t FlvMux::get_current_time(void)
        {
            return current_tag_time_;
        }

        boost::uint32_t FlvMux::video_track_index(void)
        {
            return video_index_;
        }

        boost::uint32_t FlvMux::audio_track_index(void)
        {
            return audio_index_;
        }

    } // namespace mux
} // namespace ppbox
