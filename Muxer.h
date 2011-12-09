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

    namespace mux
    {

        class Muxer
        {
        public:
            Muxer()
                : demuxer_(NULL)
                , paused_(false)
                , play_time_(0)
                , read_step_(0)
                , demux_filter_(media_info_)
                , key_filter_(media_info_)
            {
                filters_.push_back(&demux_filter_);
                filters_.push_back(&key_filter_);
            }

            virtual ~Muxer()
            {
                if (demuxer_ != NULL) {
                    // demuxer具体的析构不在mux内里实现
                    demuxer_ = NULL;
                }
            }

        protected:
            virtual void add_stream(
                MediaInfoEx & mediainfo,
                std::vector<Transfer *> & transfer) = 0;

            virtual void file_header(ppbox::demux::Sample & tag) = 0;

            virtual void stream_header(
                boost::uint32_t index, 
                ppbox::demux::Sample & tag
                ) = 0;

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

            framework::configure::Config & Config();

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

            boost::system::error_code mediainfo_translater(
                MediaInfoEx & stream_info,
                boost::system::error_code & ec);

            void create_transfer(void);

            void release_transfer(void);

            void release_decode(void);

        private:
            demux::BufferDemuxer * demuxer_;
            framework::container::List<Filter> filters_;
            bool paused_;
            boost::uint32_t play_time_; // ms
            MediaFileInfo media_info_;
            // For reset
            boost::uint32_t read_step_;
            std::vector<std::vector<Transfer *> > stream_transfers_;
            DemuxFilter demux_filter_;
            KeyFrameFilter key_filter_;

            framework::configure::Config config_;
        };
    }
}

#endif // _PPBOX_MUX_MUXER_H_
