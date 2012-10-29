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

    namespace cdn
    {
         struct DurationInfo;
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
            boost::system::error_code open(
                demux::SegmentDemuxer * demuxer,
                boost::system::error_code & ec);

            boost::system::error_code read(
                Sample & tag,
                boost::system::error_code & ec);

            void reset(void);

            bool is_open();

            virtual boost::system::error_code time_seek(
                boost::uint64_t & time,
                boost::system::error_code & ec);

            virtual boost::system::error_code byte_seek(
                boost::uint64_t & offset,
                boost::system::error_code & ec);

            void close(void);

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
                Sample & tag) = 0;

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & tag) = 0;

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
            boost::system::error_code get_sample(
                Sample & sample,
                boost::system::error_code & ec);

            boost::system::error_code get_sample_with_transfer(
                Sample & sample,
                boost::system::error_code & ec);

            boost::system::error_code open_impl(
                boost::system::error_code & ec);

            void release_info(void);

        private:
            static std::map<std::string, MuxerBase::register_type> & muxer_map();

        protected:
            MediaInfo media_info_;
            std::vector<StreamInfo> streams_;

        private:
            ppbox::demux::SegmentDemuxer * demuxer_;
            framework::container::List<Filter> filters_;
            std::vector<std::vector<Transfer *> > transfers_;

            std::string format_;
            bool paused_;
            boost::uint64_t play_time_; // ms
            // For reset 
            boost::uint32_t read_step_;
            DemuxerFilter demux_filter_;
            KeyFrameFilter key_filter_;

            framework::configure::Config config_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MUXER_BASE_H_
