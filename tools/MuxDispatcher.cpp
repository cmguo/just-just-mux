// RtspSession.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/tools/MuxDispatcher.h"
#include "ppbox/mux/MuxerModule.h"
#include "ppbox/mux/MuxError.h"



#include <ppbox/demux/DemuxerModule.h>
#include <ppbox/demux/base/BufferDemuxer.h>
#include <ppbox/demux/base/DemuxerError.h>
#include <ppbox/demux/base/SourceError.h>

#include <framework/logger/StreamRecord.h>
using namespace framework::system::logic_error;
using namespace framework::logger;

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
using namespace boost::system;

//FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("MuxDispatcher", 0);

namespace ppbox
{
    namespace mux
    {
        MuxDispatcher::MuxDispatcher(
            util::daemon::Daemon & daemon)
            : ppbox::common::Dispatcher(daemon)
            , timer_(daemon.io_svc())
            , demuxer_module_(util::daemon::use_module<ppbox::demux::DemuxerModule>(daemon))
            , muxer_module_(util::daemon::use_module<MuxerModule>(daemon))
            , demux_close_token_(0)
            , mux_close_token_(0)
            , muxer_(NULL)
            , demuxer_(NULL)
            , player_(new MuxPlayer())
        {
        }

        MuxDispatcher::~MuxDispatcher()
        {
            if(player_)
            {
                assert(!player_->is_working());
                delete player_;
                player_ = NULL;
            }
        }

        void MuxDispatcher::set_host(std::string const & host)
        {
            assert(NULL != muxer_);
            if(NULL != muxer_)
            {
                muxer_->config().set("M3U8","full_path", host);
            }
        }

        void MuxDispatcher::async_open_playlink(std::string const &playlink,ppbox::common::session_callback_respone const &resp)
        {
            demuxer_module_.async_open(
                playlink,
                demux_close_token_,
                boost::bind(&MuxDispatcher::open_call_back_mux,this,resp, _1, _2));
        }

        void MuxDispatcher::open_call_back_mux(
            ppbox::common::session_callback_respone const &resp,
            boost::system::error_code const & ec,
            ppbox::demux::BufferDemuxer * muxer)
        {
            demuxer_ = muxer;
            resp(ec);
        }

        void MuxDispatcher::cancel_open_playlink(boost::system::error_code& ec)
        {
            demuxer_module_.close(demux_close_token_,ec);
            demux_close_token_ = 0;
            demuxer_ = NULL;
        }

        void MuxDispatcher::close_playlink(boost::system::error_code& ec)
        {
            demuxer_module_.close(demux_close_token_,ec);
            demux_close_token_ = 0;
            demuxer_ = NULL;
        }

        void MuxDispatcher::open_format(std::string const &format,boost::system::error_code& ec)
        {
            muxer_ = muxer_module_.open(
                demuxer_,
                format,
                mux_close_token_);
        }

        void MuxDispatcher::close_format(boost::system::error_code& ec)
        {
            muxer_module_.close(mux_close_token_,ec);
            mux_close_token_ = 0;
            muxer_ = NULL;
        }

        void MuxDispatcher::pause_moive(boost::system::error_code& ec)
        {
            player_->pause();
        }

        void MuxDispatcher::resume_moive(boost::system::error_code& ec)
        {
            player_->resume();
        }

        void MuxDispatcher::async_play_playlink(ppbox::common::Session* session,ppbox::common::session_callback_respone const &resp)
        {
            player_->set(muxer_,resp,session,demuxer_);

            post(player_);
        }

        void MuxDispatcher::cancel_play_playlink(boost::system::error_code& ec)
        {
            player_->stop();
        }

        void MuxDispatcher::async_buffering(ppbox::common::Session* session,ppbox::common::session_callback_respone const &resp)
        {
            player_->set(muxer_,resp);
            post(player_);
        }

        void MuxDispatcher::cancel_buffering(boost::system::error_code& ec)
        {
            player_->stop();
        }


        void MuxDispatcher::async_wait(
            boost::uint32_t wait_timer
            , ppbox::common::session_callback_respone const &resp) 
        {
            //time
            timer_.expires_from_now(boost::posix_time::seconds(wait_timer));
            timer_.async_wait(resp);
        }

        void MuxDispatcher::cancel_wait(boost::system::error_code& ec)
        {
            timer_.cancel();
        }

        boost::system::error_code MuxDispatcher::get_media_info(
            ppbox::common::MediaInfo & info)
        {
             ppbox::mux::MediaFileInfo& infoTemp = muxer_->mediainfo();
             info.filesize = infoTemp.filesize;
             info.attachment = infoTemp.attachment;
            return boost::system::error_code();
        }

        boost::system::error_code MuxDispatcher::get_play_info(
            ppbox::common::PlayInfo & info)
        {
            return boost::system::error_code();
        }

    } // namespace mux
} // namespace ppbox
