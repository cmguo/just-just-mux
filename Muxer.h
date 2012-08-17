// Muxer.h
#ifndef _PPBOX_MUX_MUXER_H_
#define _PPBOX_MUX_MUXER_H_

#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/MuxerType.h"
#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"

#include <framework/configure/Config.h>

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

        class Muxer
        {
        public:
            Muxer();

            virtual ~Muxer();

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
                ppbox::common::DurationInfo & info, 
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
    }
}

#endif // _PPBOX_MUX_MUXER_H_
