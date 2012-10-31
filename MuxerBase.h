// MuxerBase.h

#ifndef _PPBOX_MUX_MUXER_BASE_H_
#define _PPBOX_MUX_MUXER_BASE_H_

#include "ppbox/mux/MuxBase.h"
#include "ppbox/mux/filter/DemuxerFilter.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"

#include <ppbox/common/Call.h>
#include <ppbox/common/Create.h>

#include <framework/configure/Config.h>

#define PPBOX_REGISTER_MUXER(n, c) \
    static ppbox::common::Call reg ## n(ppbox::mux::MuxerBase::register_muxer, BOOST_PP_STRINGIZE(n), ppbox::common::Creator<c>())

namespace ppbox
{
    namespace demux
    {
        class SegmentDemuxer;
    }

    namespace mux
    {

        class Transfer;

        class MuxerBase
        {
        public:
            typedef boost::function<
                MuxerBase * (void)
            > register_type;

        public:
            static void register_muxer(
                std::string const & format,
                register_type func);

            static MuxerBase * create(
                std::string const & format);

            static void destory(
                MuxerBase* & muxer);

        public:
            MuxerBase();

            virtual ~MuxerBase();

        public:
            bool open(
                ppbox::demux::SegmentDemuxer * demuxer, 
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
            framework::configure::Config & config()
            {
                return config_;
            }

            virtual void media_info(
                MediaInfo & info) const;

        protected:
            virtual void add_stream(
                StreamInfo & info) = 0;

            virtual void file_header(
                Sample & sample) = 0;

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & sample) = 0;

        protected:
            ppbox::demux::SegmentDemuxer const & demuxer() const
            {
                return *demuxer_;
            };

            void add_filter(
                Filter & filter)
            {
                filters_.push_back(&filter);
            }

            void add_transfer(
                boost::uint32_t index, 
                Transfer & transfer)
            {
                transfers_[index].push_back(&transfer);
            }

        private:
            void get_sample(
                Sample & sample,
                boost::system::error_code & ec);

            void on_seek(
                boost::uint64_t time);

            void open(
                boost::system::error_code & ec);

            void close();

        private:
            static std::map<std::string, MuxerBase::register_type> & muxer_map();

        protected:
            MediaInfo media_info_;
            std::vector<StreamInfo> streams_;

        private:
            ppbox::demux::SegmentDemuxer * demuxer_;
            framework::container::List<Filter> filters_;
            std::vector<std::vector<Transfer *> > transfers_;

            enum FlagEnum
            {
                f_head = 1, // 头部没有输出
                f_seek = 2, // 拖动没有完成
            };

            std::string format_;
            boost::uint64_t play_time_; // ms
            boost::uint32_t read_flag_;
            boost::uint32_t head_step_;
            DemuxerFilter demux_filter_;
            KeyFrameFilter key_filter_;

            framework::configure::Config config_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MUXER_BASE_H_
