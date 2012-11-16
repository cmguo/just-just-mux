// MuxerBase.h

#ifndef _PPBOX_MUX_MUXER_BASE_H_
#define _PPBOX_MUX_MUXER_BASE_H_

#include "ppbox/mux/MuxBase.h"
#include "ppbox/mux/filter/DemuxerFilter.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"

#include <ppbox/common/ClassFactory.h>

#include <framework/configure/Config.h>

namespace ppbox
{
    namespace mux
    {

        class Transfer;

        class MuxerBase
            : public ppbox::common::ClassFactory<
                MuxerBase, 
                std::string, 
                MuxerBase * ()
            >
        {
        public:
            MuxerBase();

            virtual ~MuxerBase();

        public:
            static MuxerBase * create(
                std::string const & format);

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

            bool close(
                boost::system::error_code & ec);

        public:
            virtual void media_info(
                MediaInfo & info) const;

            virtual void play_info(
                PlayInfo & info) const;

        public:
            framework::configure::Config & config()
            {
                return config_;
            }

        protected:
            virtual void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers) = 0;

            virtual void file_header(
                Sample & sample) = 0;

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & sample) = 0;

        protected:
            ppbox::demux::DemuxerBase const & demuxer() const
            {
                return *demuxer_;
            };

            void add_filter(
                Filter & filter)
            {
                filters_.push_back(&filter);
            }

            void reset_header(
                bool file_header = true, 
                bool stream_header = true);

        private:
            void open(
                boost::system::error_code & ec);

            void get_sample(
                Sample & sample,
                boost::system::error_code & ec);

            void on_seek(
                boost::uint64_t time);

            void close();

        protected:
            MediaInfo media_info_;
            std::vector<StreamInfo> streams_;

        private:
            ppbox::demux::DemuxerBase * demuxer_;
            framework::container::List<Filter> filters_;
            std::vector<std::vector<Transfer *> > transfers_;

            enum FlagEnum
            {
                f_head = 1, // 头部没有输出
                f_seek = 2, // 拖动没有完成
            };

            std::string format_;
            boost::uint64_t seek_time_; // ms
            boost::uint64_t play_time_; // ms
            boost::uint32_t read_flag_;
            boost::uint32_t head_step_;
            DemuxerFilter demux_filter_;
            KeyFrameFilter key_filter_;

            framework::configure::Config config_;
        };

    } // namespace mux
} // namespace ppbox

#define PPBOX_REGISTER_MUXER(k, c) PPBOX_REGISTER_CLASS(k, c)

#endif // _PPBOX_MUX_MUXER_BASE_H_
