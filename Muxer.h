// Muxer.h

#ifndef _PPBOX_MUX_MUXER_H_
#define _PPBOX_MUX_MUXER_H_

#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/Filter.h"
#include "ppbox/mux/FilterPipe.h"

#include <ppbox/avformat/SeekPoint.h>

#include <ppbox/data/base/MediaInfo.h>
#include <ppbox/data/base/StreamStatus.h>

#include <ppbox/common/ClassFactory.h>

namespace ppbox
{
    namespace avformat
    {
        class Format;
    }

    namespace mux
    {

        class FilterManager;
        class KeyFrameFilter;
        class MergeHook;

        class Muxer
            : public MuxerBase
            , public ppbox::common::ClassFactory<
                Muxer, 
                std::string, 
                Muxer * ()
            >
        {
        public:
            Muxer();

            virtual ~Muxer();

        public:
            static Muxer * create(
                std::string const & foramt);

        public:
            bool open(
                ppbox::demux::DemuxerBase * demuxer, 
                boost::system::error_code & ec);

            virtual bool setup(
                boost::uint32_t index, 
                boost::system::error_code & ec);

            bool read(
                Sample & sample,
                boost::system::error_code & ec);

            bool reset(
                boost::system::error_code & ec);

            virtual bool time_seek(
                boost::uint64_t & offset,
                boost::system::error_code & ec);

            virtual bool byte_seek(
                boost::uint64_t & offset,
                boost::system::error_code & ec);

            virtual boost::uint64_t check_seek(
                boost::system::error_code & ec);

            bool close(
                boost::system::error_code & ec);

        public:
            virtual void media_info(
                MediaInfo & info) const;

            virtual void stream_info(
                std::vector<StreamInfo> & streams) const;

            virtual void stream_status(
                StreamStatus & info) const;

        protected:
            virtual void do_open(
                MediaInfo & info) {};

            virtual void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe) = 0;

            virtual void file_header(
                Sample & sample) = 0;

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & sample) = 0;

            virtual void do_close() {};

        protected:
            ppbox::demux::DemuxerBase const & demuxer() const
            {
                return *demuxer_;
            };

            void format(
                ppbox::avformat::Format * format);

            void format(
                std::string const & format);

            void add_filter(
                Filter * filter, 
                bool adopt = true);

            void reset_header(
                bool file_header = true, 
                bool stream_header = true);

            void get_seek_points(
                std::vector<ppbox::avformat::SeekPoint> & points);

        private:
            void open(
                boost::system::error_code & ec);

            void get_sample(
                Sample & sample,
                boost::system::error_code & ec);

            void after_seek(
                boost::uint64_t time);

            void close();

        protected:
            MediaInfo media_info_;
            std::vector<StreamInfo> streams_;
            StreamStatus stat_;

        private:
            ppbox::demux::DemuxerBase * demuxer_;
            FilterManager * manager_;

            enum FlagEnum
            {
                f_head = 1, // 头部没有输出
                f_seek = 2, // 拖动没有完成
            };

            std::string format_str_;
            ppbox::avformat::Format * format_;
            std::string video_codec_;
            std::string audio_codec_;
            std::string debug_codec_;
            bool pseudo_seek_;
            boost::uint32_t read_flag_;
            boost::uint32_t head_step_;
            KeyFrameFilter * key_filter_;
        };

    } // namespace mux
} // namespace ppbox

#define PPBOX_REGISTER_MUXER(k, c) PPBOX_REGISTER_CLASS(k, c)

#endif // _PPBOX_MUX_MUXER_BASE_H_
