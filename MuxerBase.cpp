// Muxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/Transfer.h"
#include "ppbox/mux/MuxError.h"
#include "ppbox/mux/FilterManager.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/filter/CodecEncoderFilter.h"
#include "ppbox/mux/transfer/CodecSplitterTransfer.h"
#include "ppbox/mux/transfer/CodecAssemblerTransfer.h"
#include "ppbox/mux/transfer/CodecDebugerTransfer.h"
#include "ppbox/mux/transfer/TimeScaleTransfer.h"

#include <ppbox/demux/base/DemuxerBase.h>
#include <ppbox/demux/base/DemuxError.h>

#include <ppbox/avformat/Format.h>
using namespace ppbox::avformat;

#include <util/buffers/BuffersSize.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.mux.MuxerBase", framework::logger::Debug);

        MuxerBase * MuxerBase::create(
            std::string const & format)
        {
            MuxerBase * muxer = factory_type::create(format);
            if (muxer) {
                if (muxer->format_str_.empty()) {
                    muxer->format_str_ = format;
                }
            }
            return muxer;
        }

        MuxerBase::MuxerBase()
            : demuxer_(NULL)
            , format_(NULL)
            , read_flag_(0)
            , head_step_(0)
        {
            config().register_module("Muxer")
                << CONFIG_PARAM_NAME_RDWR("video_codec", video_codec_)
                << CONFIG_PARAM_NAME_RDWR("audio_codec", audio_codec_)
                << CONFIG_PARAM_NAME_RDWR("debug_codec", debug_codec_)
                ;

            manager_ = new FilterManager;
            key_filter_ = new KeyFrameFilter;
        }

        MuxerBase::~MuxerBase()
        {
            if (demuxer_ != NULL) {
                // demuxer具体的析构不在mux内里实现
                demuxer_ = NULL;
            }
            delete key_filter_;
            delete manager_;
            if (format_)
                delete format_;
        }

        bool MuxerBase::open(
            ppbox::demux::DemuxerBase * demuxer,
            error_code & ec)
        {
            assert(demuxer != NULL);
            demuxer_ = demuxer;
            open(ec);
            if (ec) {
                demuxer_ = NULL;
            } else {
                read_flag_ = f_head;
                stat_.time_range.beg = stat_.time_range.pos = demuxer_->check_seek(ec);
                if (ec == boost::asio::error::would_block) {
                    ec.clear();
                    read_flag_ = f_seek;
                }
            }
            return !ec;
        }

        bool MuxerBase::setup(
            boost::uint32_t index, 
            boost::system::error_code & ec)
        {
            if (index != (boost::uint32_t)-1) {
                ec == framework::system::logic_error::not_supported;
            } else {
                ec.clear();
            }
            return !ec;
        }

        bool MuxerBase::read(
            Sample & sample,
            error_code & ec)
        {
            if (read_flag_) {
                if (read_flag_ & f_head) {
                    sample.itrack = (boost::uint32_t)-1;
                    sample.time = 0;
                    sample.dts = 0;
                    sample.cts_delta = 0;
                    sample.duration = 0;
                    sample.size = 0;
                    sample.stream_info = NULL;
                    sample.data.clear();
                    do {
                        if (head_step_ == 0) {
                            file_header(sample);
                            ++head_step_;
                        } else {
                            assert(head_step_ <= streams_.size());
                            stream_header(head_step_ - 1, sample);
                            if (head_step_ == streams_.size()) {
                                head_step_ = 0;
                                read_flag_ &= ~f_head;
                            } else {
                                ++head_step_;
                            }
                        }
                    } while (sample.data.empty() && (read_flag_ & f_head));
                    if (!sample.data.empty()) {
                        sample.size = util::buffers::buffers_size(sample.data);
                        ec.clear();
                        stat_.byte_range.pos += sample.size;
                        return true;
                    }
                } else if (read_flag_ & f_seek) {
                    boost::uint64_t time = demuxer_->check_seek(ec);
                    if (!ec) {
                        after_seek(time);
                        read_flag_ &= ~f_seek;
                    } else {
                        return false;
                    }
                }
            }

            get_sample(sample, ec);

            return !ec;
        }

        bool MuxerBase::reset(
            error_code & ec)
        {
            if (!manager_->begin_reset(ec)) {
                return false;
            }
            demuxer_->reset(ec);
            if (!ec) {
                manager_->reset(ec);
                read_flag_ |= f_head;
                boost::uint64_t time = demuxer_->check_seek(ec);
                if (!ec) {
                    after_seek(time);
                }
            } else if (ec ==boost::asio::error::would_block) {
                read_flag_ |= f_head;
                read_flag_ |= f_seek;
            }
            return !ec;
        }

        bool MuxerBase::time_seek(
            boost::uint64_t & time,
            error_code & ec)
        {
            if (!manager_->begin_seek(time, ec)) {
                return false;
            }
            demuxer_->seek(time, ec);
            if (!ec) {
                manager_->reset(ec);
                read_flag_ |= f_head;
                after_seek(time);
            } else if (ec == boost::asio::error::would_block) {
                manager_->reset(ec);
                stat_.time_range.beg = time;
                read_flag_ |= f_head;
                read_flag_ |= f_seek;
            }
            stat_.byte_range.beg = 0;
            stat_.byte_range.end = ppbox::data::invalid_size;
            stat_.byte_range.pos = 0;
            stat_.byte_range.buf = 0;
            return !ec;
        }

        bool MuxerBase::byte_seek(
            boost::uint64_t & offset,
            boost::system::error_code & ec)
        {
            boost::uint64_t seek_time = (offset * media_info_.duration) / media_info_.file_size;
            return time_seek(seek_time, ec);
        }

        boost::uint64_t MuxerBase::check_seek(
            boost::system::error_code & ec)
        {
            if (read_flag_ & f_seek) {
                boost::uint64_t time = demuxer_->check_seek(ec);
                if (!ec) {
                    after_seek(time);
                    read_flag_ &= ~f_seek;
                }
            } else {
                ec.clear();
            }
            return stat_.time_range.beg;
        }

        void MuxerBase::media_info(
            MediaInfo & info) const
        {
            boost::system::error_code ec;
            demuxer_->get_media_info(info, ec);
            info.file_size = ppbox::data::invalid_size;
            info.format = format_str_;
        }

        void MuxerBase::stream_info(
            std::vector<StreamInfo> & streams) const
        {
            streams = streams_;
        }

        void MuxerBase::stream_status(
            StreamStatus & status) const
        {
            boost::system::error_code ec;
            StreamStatus status1;
            demuxer_->get_stream_status(status1, ec);
            status1.byte_range = stat_.byte_range;
            status1.time_range.beg = stat_.time_range.beg;
            status = status1;
        }

        bool MuxerBase::close(
            boost::system::error_code & ec)
        {
            close();
            demuxer_ = NULL;
            ec.clear();
            return true;
        }

        void MuxerBase::format(
            std::string const & format)
        {
            format_ = Format::create(format);
        }

        void MuxerBase::format(
            ppbox::avformat::Format * format)
        {
            format_ = format;
        }

        void MuxerBase::add_filter(
            Filter * filter, 
            bool adopt)
        {
            boost::system::error_code ec;
            manager_->append_filter(filter, adopt, ec);
        }

        void MuxerBase::reset_header(
            bool file_header, 
            bool stream_header)
        {
            read_flag_ |= f_head;
            head_step_ = file_header ? 0 : 1;
        }

        void MuxerBase::open(
            boost::system::error_code & ec)
        {
            assert(demuxer_ != NULL);
            demuxer_->get_media_info(media_info_, ec);
            if (ec) {
                return;
            }
            do_open(media_info_);
            size_t stream_count = demuxer_->get_stream_count(ec);
            streams_.resize(stream_count);
            manager_->open(demuxer_, stream_count, ec);
            manager_->append_filter(key_filter_, false, ec);
            if (format_ == NULL) {
                format_ = Format::create(format_str_);
            }
            boost::uint32_t video_codec = StreamType::from_string(video_codec_);
            boost::uint32_t audio_codec = StreamType::from_string(audio_codec_);
            boost::uint32_t debug_codec = StreamType::from_string(debug_codec_);
            for (size_t i = 0; i < stream_count; ++i) {
                StreamInfo & stream = streams_[i];
                FilterPipe & pipe = manager_->pipe(i);
                demuxer_->get_stream_info(i, stream, ec);
                if (ec) {
                    break;
                }
                StreamInfo info = stream;
                CodecInfo const * codec = format_->codec_from_codec(info.type, info.sub_type);
                if (stream.type == StreamType::VIDE && video_codec && video_codec != stream.sub_type) {
                    LOG_INFO("[open] change video codec from " << StreamType::to_string(stream.sub_type) << " to " << video_codec_);
                    info.sub_type = video_codec;
                    codec = format_->codec_from_codec(info.type, info.sub_type);
                    if (codec) {
                        info.format_type = codec->codec_format;
                        pipe.insert(new CodecEncoderFilter(info));
                        manager_->remove_filter(key_filter_, ec);
                        if (debug_codec == info.sub_type) {
                            LOG_INFO("[open] add debuger of codec " << StreamType::to_string(info.sub_type));
                            pipe.insert(new CodecDebugerTransfer(info.sub_type));
                        }
                    } else {
                        LOG_ERROR("[open] video codec " << video_codec_ << " not supported by cantainer " << format_str_);
                        ec = error::format_not_support;
                    }
                } else if (stream.type == StreamType::AUDI && audio_codec && audio_codec != stream.sub_type) {
                    LOG_INFO("[open] change audio codec from " << StreamType::to_string(stream.sub_type) << " to " << audio_codec_);
                    info.sub_type = audio_codec;
                    codec = format_->codec_from_codec(info.type, info.sub_type);
                    if (codec) {
                        info.format_type = codec->codec_format;
                        pipe.insert(new CodecEncoderFilter(info));
                        if (debug_codec == info.sub_type) {
                            LOG_INFO("[open] add debuger of codec " << StreamType::to_string(info.sub_type));
                            pipe.insert(new CodecDebugerTransfer(info.sub_type));
                        }
                    } else {
                        LOG_ERROR("[open] audio codec " << audio_codec_ << " not supported by cantainer " << format_str_);
                        ec = error::format_not_support;
                    }
                } else if (codec) {
                    if (codec->codec_format != info.format_type || debug_codec == info.sub_type) {
                        LOG_INFO("[open] change format of codec " << StreamType::to_string(info.sub_type) << " from " << info.format_type << " to " << codec->codec_format);
                        if (info.format_type) {
                            pipe.insert(new CodecSplitterTransfer(info.sub_type, info.format_type));
                        }
                        if (debug_codec == info.sub_type) {
                            LOG_INFO("[open] add debuger of codec " << StreamType::to_string(info.sub_type));
                            pipe.insert(new CodecDebugerTransfer(info.sub_type));
                        }
                        if (codec->codec_format && codec->codec_format != info.format_type) {
                            pipe.insert(new CodecAssemblerTransfer(info.sub_type, codec->codec_format));
                        }
                        info.format_type = codec->codec_format;
                    }
                } else {
                    LOG_ERROR("[open] codec " << StreamType::to_string(info.sub_type) << " not supported by cantainer " << format_str_);
                    ec = error::format_not_support;
                }
                if (codec && info.time_scale != codec->time_scale) {
                    info.time_scale = codec->time_scale;
                    if (codec->time_scale != 0 && codec->time_scale != 1000) {
                        LOG_INFO("[open] change time scale from " << info.time_scale << " to " << codec->time_scale);
                        pipe.insert(new TimeScaleTransfer(codec->time_scale));
                    }
                }
                if (ec) break;
                add_stream(info, pipe);
            }
            if (!ec) {
                manager_->complete(config_, streams_, ec);
            }
        }

        void MuxerBase::after_seek(
            boost::uint64_t time)
        {
            stat_.time_range.beg = time;
            stat_.time_range.pos = time;
            error_code ec;
            manager_->finish_seek(time, ec);
        }

        void MuxerBase::get_sample(
            Sample & sample,
            error_code & ec)
        {
            if (manager_->pull_one(sample, ec)) {
                //if (sample.flags & Sample::stream_changed) {
                    //release_info();
                    //open_impl(ec);
                    //read_flag_ |= f_head;
                    //head_step_ = 1;
                //}
                LOG_TRACE("[get_sample] itrack: " << sample.itrack << " time: " << sample.time << " dts: " << sample.dts);
                stat_.time_range.pos = sample.time;
                stat_.byte_range.pos += sample.size;
            } else if (ec == error::end_of_stream) {
                stat_.byte_range.end = stat_.byte_range.pos;
            }
        }

        void MuxerBase::close()
        {
            error_code ec;
            manager_->reset(ec);
            manager_->remove_filter(key_filter_, ec);
            do_close();
            manager_->close(ec);
            streams_.clear();
        }

    } // namespace mux
} // namespace ppbox
