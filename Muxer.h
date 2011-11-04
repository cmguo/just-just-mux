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
        class Demuxer;
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
                , is_read_head_(false)
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
                ppbox::demux::MediaInfo & mediainfo,
                std::vector<Transfer *> & transfer) = 0;

            virtual void head_buffer(ppbox::demux::Sample & tag) = 0;

        public:
            virtual boost::system::error_code open(
                demux::Demuxer * demuxer,
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
            ppbox::demux::Sample & get_sample(
                boost::system::error_code & ec);

            ppbox::demux::Sample & get_sample_with_transfer(
                boost::system::error_code & ec);

            boost::system::error_code mediainfo_translater(
                ppbox::demux::MediaInfo & stream_info,
                boost::system::error_code & ec);

            void transfer_sample(ppbox::demux::Sample & sample);

            void create_transfer(void);

            void release_transfer(void);

        private:
            demux::Demuxer * demuxer_;
            framework::container::List<Filter> filters_;
            bool paused_;
            boost::uint32_t play_time_; // ms
            MediaFileInfo media_info_;
            // For reset
            bool is_read_head_;
            std::vector<Transfer *> video_transfers_;
            std::vector<Transfer *> audio_transfers_;
            ppbox::demux::Sample sample_;
            DemuxFilter demux_filter_;
            KeyFrameFilter key_filter_;

            std::string attachment_;
            framework::configure::Config config_;
        };
    }
}

#endif // _PPBOX_MUX_MUXER_H_
