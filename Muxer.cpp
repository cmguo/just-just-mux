// Muxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/Transfer.h"
#include "ppbox/mux/MuxError.h"
#include "ppbox/mux/FilterManager.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/filter/TranscodeFilter.h"
#include "ppbox/mux/transfer/CodecSplitterTransfer.h"
#include "ppbox/mux/transfer/CodecAssemblerTransfer.h"
#include "ppbox/mux/transfer/CodecDebugerTransfer.h"
#include "ppbox/mux/transfer/TimeScaleTransfer.h"

#include <ppbox/demux/base/DemuxerBase.h>
#include <ppbox/demux/base/DemuxError.h>

#include <ppbox/avformat/Format.h>
using namespace ppbox::avformat;

#include <util/buffers/BuffersSize.h>

#include <framework/string/Slice.h>
#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.mux.Muxer", framework::logger::Debug);


        Muxer::Muxer()
            : demuxer_(NULL)
            , format_(NULL)
            , pseudo_seek_(false)
            , read_flag_(0)
            , head_step_(0)
        {
            config().register_module("Muxer")
                << CONFIG_PARAM_NAME_RDWR("video_codec", video_codec_)
                << CONFIG_PARAM_NAME_RDWR("audio_codec", audio_codec_)
                << CONFIG_PARAM_NAME_RDWR("debug_codec", debug_codec_)
                << CONFIG_PARAM_NAME_RDWR("pseudo_seek", pseudo_seek_)
                ;

            manager_ = new FilterManager;
            key_filter_ = new KeyFrameFilter;
        }

        Muxer::~Muxer()
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

        bool Muxer::open(
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

        bool Muxer::setup(
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

        bool Muxer::read(
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

        /* reset from begin
         * Events:
         *   begin_reset (if false stop reset)
         *   reset
         *   finish_seek
         */ 
        bool Muxer::reset(
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

        /* seek to time
         * Events:
         *   begin_seek (if false stop reset)
         *   reset
         *   finish_seek
         */ 
        bool Muxer::time_seek(
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

        bool Muxer::byte_seek(
            boost::uint64_t & offset,
            boost::system::error_code & ec)
        {
            boost::uint64_t seek_time = (offset * media_info_.duration) / media_info_.file_size;
            return time_seek(seek_time, ec);
        }

        boost::uint64_t Muxer::check_seek(
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

        void Muxer::media_info(
            MediaInfo & info) const
        {
            boost::system::error_code ec;
            demuxer_->get_media_info(info, ec);
            info.file_size = ppbox::data::invalid_size;
            info.format = format_str_;
        }

        void Muxer::stream_info(
            std::vector<StreamInfo> & streams) const
        {
            streams = streams_;
        }

        void Muxer::stream_status(
            StreamStatus & status) const
        {
            boost::system::error_code ec;
            StreamStatus status1;
            demuxer_->get_stream_status(status1, ec);
            status1.byte_range = stat_.byte_range;
            status1.time_range.beg = stat_.time_range.beg;
            status = status1;
        }

        bool Muxer::close(
            boost::system::error_code & ec)
        {
            close();
            demuxer_ = NULL;
            ec.clear();
            return true;
        }

        void Muxer::format(
            std::string const & format)
        {
            boost::system::error_code ec;
            format_ = FormatFactory::create(format, ec);
        }

        void Muxer::format(
            ppbox::avformat::Format * format)
        {
            format_ = format;
        }

        void Muxer::add_filter(
            Filter * filter, 
            bool adopt)
        {
            boost::system::error_code ec;
            manager_->append_filter(filter, adopt, ec);
        }

        /* start new file from current
         * Events:
         *   reset
         */ 
        void Muxer::reset_header(
            bool file_header, 
            bool stream_header)
        {
            read_flag_ |= f_head;
            head_step_ = file_header ? 0 : 1;
            boost::system::error_code ec;
            manager_->reset(ec);
        }

        void Muxer::get_seek_points(
            std::vector<ppbox::avformat::SeekPoint> & points)
        {
            if (pseudo_seek_ && media_info_.bitrate && media_info_.duration != ppbox::data::invalid_size) {
                boost::uint64_t time_interval = 10 * 1000; // 10 seconds
                boost::uint64_t offset_interval = time_interval * media_info_.bitrate / 8 / 1000;
                points.resize((size_t)(media_info_.duration / time_interval + 1));
                boost::uint64_t time = 0;
                boost::uint64_t offset = 0;
                for (size_t i = 0; time < media_info_.duration; time += time_interval, offset += offset_interval, ++i) {
                    points[i].time = time;
                    points[i].offset = offset;
                }
            }
        }

        static std::vector<boost::uint32_t> codecs_from_string(
            std::string const & codecs_str)
        {
            std::vector<std::string> codec_strs;
            framework::string::slice<std::string>(codecs_str, std::back_inserter(codec_strs));
            std::vector<boost::uint32_t> codecs;
            for (size_t i = 0; i < codec_strs.size(); ++i) {
                codecs.push_back(ppbox::avbase::FourCC::from_string(codec_strs[i]));
            }
            return codecs;
        }

        static bool codec_in(
            boost::uint32_t codec, 
            std::vector<boost::uint32_t> const & codecs)
        {
            return std::find(codecs.begin(), codecs.end(), codec) != codecs.end();
        }

        void Muxer::open(
            boost::system::error_code & ec)
        {
            using ppbox::avbase::FourCC;

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
                format_ = FormatFactory::create(format_str_, ec);
            }
            std::vector<boost::uint32_t> video_codecs = codecs_from_string(video_codec_);
            std::vector<boost::uint32_t> audio_codecs = codecs_from_string(audio_codec_);
            std::vector<boost::uint32_t> debug_codecs = codecs_from_string(debug_codec_);
            std::vector<boost::uint32_t> empty_codecs;
            for (size_t i = 0; i < stream_count; ++i) {
                StreamInfo & stream = streams_[i];
                demuxer_->get_stream_info(i, stream, ec);
                if (ec) {
                    break;
                }
                LOG_INFO("[open] stream index: " << i 
                    << " type: " << FourCC::to_string(stream.type)
                    << " sub_type: " << FourCC::to_string(stream.sub_type));
                FilterPipe & pipe = manager_->pipe(i);
                StreamInfo tempstream = stream;
                std::vector<boost::uint32_t> const & output_codecs = 
                    stream.type == StreamType::VIDE ? video_codecs : 
                    (stream.type == StreamType::AUDI ? audio_codecs : empty_codecs);
                if (!output_codecs.empty() && !codec_in(stream.sub_type, output_codecs)) {
                    if (tempstream.format_type != ppbox::avbase::StreamFormatType::none) {
                        LOG_INFO("[open] change format of codec " << FourCC::to_string(tempstream.sub_type) 
                            << " from " << tempstream.format_type);
                        pipe.insert(new CodecSplitterTransfer(tempstream.sub_type, tempstream.format_type));
                        tempstream.format_type = ppbox::avbase::StreamFormatType::none;
                    }
                    std::auto_ptr<TranscodeFilter> tf(new TranscodeFilter(stream, output_codecs));
                    tempstream.sub_type = tf->output_codec();
                    if (tempstream.sub_type == ppbox::avbase::StreamSubType::NONE) {
                        LOG_INFO("[open] can't change codec");
                    } else {
                        LOG_INFO("[open] change codec from " << FourCC::to_string(stream.sub_type) 
                            << " to " << FourCC::to_string(tempstream.sub_type));
                        pipe.insert(tf.release());
                        manager_->remove_filter(key_filter_, ec);
                        if (codec_in(tempstream.sub_type, debug_codecs)) {
                            LOG_INFO("[open] add debuger of codec " << FourCC::to_string(tempstream.sub_type));
                            pipe.insert(new CodecDebugerTransfer(tempstream.sub_type));
                        }
                    }
                }
                CodecInfo const * codec = format_->codec_from_codec(tempstream.type, tempstream.sub_type, ec);
                if (codec) {
                    if (codec->codec_format != tempstream.format_type || codec_in(tempstream.sub_type, debug_codecs)) {
                        LOG_INFO("[open] change format of codec " << FourCC::to_string(tempstream.sub_type) 
                            << " from " << tempstream.format_type << " to " << codec->codec_format);
                        if (tempstream.format_type) {
                            pipe.insert(new CodecSplitterTransfer(tempstream.sub_type, tempstream.format_type));
                        }
                        if (codec->codec_format && codec->codec_format != tempstream.format_type) {
                            pipe.insert(new CodecAssemblerTransfer(tempstream.sub_type, codec->codec_format));
                        }
                        tempstream.format_type = codec->codec_format;
                    }
                } else {
                    LOG_ERROR("[open] codec " << FourCC::to_string(tempstream.sub_type) 
                        << " not supported by cantainer " << format_str_);
                }
                if (codec && tempstream.time_scale != codec->time_scale) {
                    tempstream.time_scale = codec->time_scale;
                    if (codec->time_scale != 0 && codec->time_scale != 1000) {
                        LOG_INFO("[open] change time scale from " << stream.time_scale << " to " << codec->time_scale);
                        pipe.insert(new TimeScaleTransfer(codec->time_scale));
                    }
                }
                if (ec) break;
                add_stream(tempstream, pipe);
            }
            if (!ec) {
                manager_->complete(config(), streams_, ec);
            }
        }

        void Muxer::after_seek(
            boost::uint64_t time)
        {
            stat_.time_range.beg = time;
            stat_.time_range.pos = time;
            error_code ec;
            manager_->finish_seek(time, ec);
        }

        void Muxer::get_sample(
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
            } else if (ec == avformat::error::end_of_stream) {
                stat_.byte_range.end = stat_.byte_range.pos;
            }
        }

        void Muxer::close()
        {
            error_code ec;
            manager_->remove_filter(key_filter_, ec);
            do_close();
            manager_->close(ec);
            streams_.clear();
        }

        boost::system::error_code MuxerTraits::error_not_found()
        {
            return error::not_support;
        }

        Muxer * MuxerFactory::create(
            std::string const & format, 
            boost::system::error_code & ec)
        {
            Muxer * muxer = factory_type::create(format, ec);
            if (muxer) {
                if (muxer->format_str_.empty()) {
                    muxer->format_str_ = format;
                }
            }
            return muxer;
        }
    } // namespace mux
} // namespace ppbox
