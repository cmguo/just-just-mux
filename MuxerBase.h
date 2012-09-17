// MuxerBase.h

#ifndef _PPBOX_MUX_MUXER_H_
#define _PPBOX_MUX_MUXER_H_

#include "ppbox/mux/MuxBase.h"
#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"

#include <ppbox/common/Call.h>
#include <ppbox/common/Create.h>

#include <framework/configure/Config.h>
#include <framework/string/Url.h>

#define PPBOX_REGISTER_MUXER(n, c) \
    static ppbox::common::Call reg ## n(ppbox::mux::MuxerBase::register_muxer, BOOST_PP_STRINGIZE(n), ppbox::common::Creator<c>())

namespace ppbox
{
    namespace demux
    {
        class BufferDemuxer;
    }

    namespace cdn
    {
         struct DurationInfo;
    }

    namespace mux
    {

        class MuxerBase
        {
        public:
            typedef boost::function<
                MuxerBase * (void)
            > register_type;

        public:
            static void register_muxer(
                std::string const & name,
                register_type func);

            static MuxerBase * create(
                std::string const & proto);

            static void destory(
                MuxerBase* & source);

        public:
            MuxerBase();

            virtual ~MuxerBase();

        protected:
            virtual void add_stream(
                MediaInfoEx & mediainfo) = 0;

            virtual void file_header(
                ppbox::demux::Sample & tag) = 0;

            virtual void stream_header(
                boost::uint32_t index, 
                ppbox::demux::Sample & tag) = 0;

        public:
            virtual boost::system::error_code open(
                demux::BufferDemuxer * demuxer,
                boost::system::error_code & ec);

            boost::system::error_code read(
                ppbox::demux::Sample & tag,
                boost::system::error_code & ec);

            void reset(void);

            bool is_open();

            virtual boost::system::error_code seek(
                boost::uint32_t & time,
                boost::system::error_code & ec);

            boost::system::error_code byte_seek(
                boost::uint32_t & offset,
                boost::system::error_code & ec);

            boost::system::error_code get_duration(
                ppbox::data::MediaInfo & info, 
                boost::system::error_code & ec);

            boost::system::error_code pause(
                boost::system::error_code & ec);

            boost::system::error_code resume(
                boost::system::error_code & ec);

            void close(void);

            virtual MediaFileInfo & mediainfo(void);

            boost::system::error_code get_buffer_time(
                boost::uint32_t & buffer_time,
                boost::system::error_code & ec);

            boost::uint32_t & current_time(void);

            framework::configure::Config & config();

        protected:
            void add_filter(Filter & filter)
            {
                filters_.push_back(&filter);
            }

        private:
            boost::system::error_code get_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

            boost::system::error_code get_sample_with_transfer(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

            boost::system::error_code open_impl(
                boost::system::error_code & ec);

            void release_mediainfo(void);

        private:
            static std::map<std::string, MuxerBase::register_type> & muxer_map();

        protected:
            MediaFileInfo media_info_;

        private:
            demux::BufferDemuxer * demuxer_;
            framework::container::List<Filter> filters_;
            bool paused_;
            boost::uint32_t play_time_; // ms
            // For reset 
            boost::uint32_t read_step_;
            DemuxFilter demux_filter_;
            KeyFrameFilter key_filter_;

            framework::configure::Config config_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MUXER_H_
