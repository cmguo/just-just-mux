// RtpMuxDispatcher.h

#ifndef _PPBOX_MUX_PATCHER_H_
#define _PPBOX_MUX_PATCHER_H_

#include "ppbox/mux/tools/MuxPlayer.h"

#include <ppbox/common/Dispatcher.h>

#include <boost/asio/deadline_timer.hpp>


namespace ppbox
{
    namespace demux
    {
        class BufferDemuxer;
        class DemuxerModule;
    }

    namespace mux
    {  
        
        class MuxerModule;
        class Muxer;

        class MuxDispatcher : public ppbox::common::Dispatcher
        {
        public:

            MuxDispatcher(
                util::daemon::Daemon & daemon);

            virtual ~MuxDispatcher();

        public:
            void set_host(std::string const & host);


            MuxPlayer* get_player()
            {
                return player_;
            }

//播放控制函数
            virtual void async_open_playlink(std::string const &playlink,ppbox::common::session_callback_respone const &resp) ;
            virtual void cancel_open_playlink(boost::system::error_code& ec) ;
            virtual void close_playlink(boost::system::error_code& ec) ;

            virtual void open_format(std::string const &format,boost::system::error_code& ec) ;
            virtual void close_format(boost::system::error_code& ec) ;

            virtual void pause_moive(boost::system::error_code& ec);
            virtual void resume_moive(boost::system::error_code& ec);

            virtual void async_play_playlink(ppbox::common::Session* session,ppbox::common::session_callback_respone const &resp);
            virtual void cancel_play_playlink(boost::system::error_code& ec);

            virtual void async_buffering(ppbox::common::Session* session,ppbox::common::session_callback_respone const &resp);
            virtual void cancel_buffering(boost::system::error_code& ec);


            virtual void async_wait(
                boost::uint32_t wait_timer
                , ppbox::common::session_callback_respone const &resp) ;

            virtual void cancel_wait(boost::system::error_code& ec) ;

            virtual boost::system::error_code get_media_info(
                ppbox::common::MediaInfo & info) ;

            virtual boost::system::error_code get_play_info(
                ppbox::common::PlayInfo & info);


        private:
            void open_call_back_mux(
                ppbox::common::session_callback_respone const &resp,
                boost::system::error_code const & ec,
                ppbox::demux::BufferDemuxer * muxer);
            
        protected:
            ppbox::mux::Muxer *muxer_;           
            ppbox::demux::BufferDemuxer* demuxer_;
            MuxPlayer* player_;

        private:
            boost::asio::deadline_timer timer_;
            ppbox::demux::DemuxerModule& demuxer_module_;
            ppbox::mux::MuxerModule& muxer_module_;

            size_t  demux_close_token_;      
            size_t  mux_close_token_;


            boost::uint32_t video_type_;          //视频索引值
            boost::uint32_t audio_type_;          //音频索引值


        };

    } // namespace rtsp
} // namespace ppbox

#endif // _PPBOX_RTSP_RTP_MuxDispatcher_H_
