// RtpDispatcher.h

#ifndef _PPBOX_MUX_PATCHER_H_
#define _PPBOX_MUX_PATCHER_H_

#include "ppbox/mux/tool/MsgQueueCommon.h"
#include "ppbox/mux/tool/Sink.h"

namespace framework { namespace thread { template <typename _Ty> class MessageQueue; } }

namespace boost { class thread; }


namespace ppbox
{
    namespace demux
    {
        class BufferDemuxer;
        class DemuxerModule;
        class PptvDemuxer;
    }

    namespace mux
    {  
        
        class MuxerModule;
        class Muxer;
        struct MediaFileInfo;

        class Dispatcher
        {
        public:

            Dispatcher(
                util::daemon::Daemon & daemon);

            virtual ~Dispatcher();

//播放控制函数
        public:
            //注册一个session_id 用于后面调用发送的一致性
            //boost::system::error_code regSrv(boost::uint32_t& id);

            boost::system::error_code open(
                boost::uint32_t& session_id,
                std::string const & play_link,
                std::string const & format,
                bool need_session,
                session_callback_respone const &);

            boost::system::error_code seek(
                const boost::uint32_t session_id
                ,const boost::uint32_t begin
                ,const boost::uint32_t end
                ,session_callback_respone const &);

            boost::system::error_code record(
                const boost::uint32_t session_id
                ,session_callback_respone const &resp);

            boost::system::error_code play(
                const boost::uint32_t session_id
                ,session_callback_respone const &resp);

            boost::system::error_code resume(
                const boost::uint32_t session_id
                ,session_callback_respone const &resp);

            //seek + play
            boost::system::error_code play(
                const boost::uint32_t session_id
                ,const boost::uint32_t begin
                ,const boost::uint32_t end
                ,session_callback_respone const &resp);

            boost::system::error_code pause(
                const boost::uint32_t session_id
                ,session_callback_respone const &resp);

           //多sink模式
            boost::system::error_code setup(
                boost::uint32_t session_id, 
                size_t  index,
                Sink*  sink,
                session_callback_respone const & resp);

            //单sink模式
            boost::system::error_code setup(
                boost::uint32_t session_id, 
                Sink*  sink,
                session_callback_respone const & resp);

            boost::system::error_code close(
                const boost::uint32_t session_id);

            boost::system::error_code stop();
            boost::system::error_code start();


            boost::system::error_code get_info(
                ppbox::mux::MediaFileInfo & info);

//状态机处理函数，根据业务不同可以由派生类实现
        protected:
            virtual boost::system::error_code thread_open(MessageQType* &param);

            virtual boost::system::error_code thread_seek(MessageQType* &param);

            virtual boost::system::error_code thread_play(MessageQType* &param);
            virtual boost::system::error_code thread_record(MessageQType* &param);
            virtual boost::system::error_code thread_resume(MessageQType* &param);
            virtual boost::system::error_code thread_pause(MessageQType* &param);
            virtual boost::system::error_code thread_close(MessageQType* &param);
            virtual boost::system::error_code thread_stop(MessageQType* &param);
            virtual boost::system::error_code thread_callback(MessageQType* &param);


            virtual boost::system::error_code thread_setup(MessageQType* &param);


            virtual boost::system::error_code thread_timeout();
        

            util::daemon::Daemon & get_daemon() 
            {
                return daemon_;
            }

//内存指令
        private:
            ppbox::demux::DemuxerModule & demuxer_module();

            MuxerModule & muxer_module();

            std::string const & status_string();

            //状态机判断函数
            boost::system::error_code thread_command(MessageQType*);

            boost::system::error_code play();
            
            //线程函数
            void thread_dispatch();

            void open_call_back_mux(
                const boost::uint32_t,
                boost::system::error_code const &,
                ppbox::demux::PptvDemuxer *);

            void clear_send(boost::system::error_code const & ec);

            struct Movie;
            void clear_movie(Movie* movie,boost::system::error_code const & ec);

        private:
            struct Movie
            {
                Movie()
                {
                    close_token = 0;
                    seek = 0;
                    mux_close_token = 0;
                    session_id = 0;
                    delay = false;
                    muxer = NULL;
                    demuxer = NULL;
                }
                
                virtual ~Movie()
                {
                    clear();
                }

                void Init(const MessageQType* msg)
                {

                    play_link = msg->play_link_;
                    format = msg->format_;
                    
                    clear();

                    seek = 0;
                    close_token = 0;
                    mux_close_token = 0;
                    session_id = 0;
                    delay = false;
                    muxer = NULL;
                    demuxer = NULL;
                }

                void clear()
                {
                    for (Iter iter = sessions.begin(); iter != sessions.end(); ++iter)
                    {
                        delete (*iter);
                    }
                    sessions.clear();

                }
                    
                std::string play_link;
                std::string format;
                boost::uint32_t session_id;

                boost::uint32_t seek;  //播放的最后时长


                size_t  close_token;               //关闭流  call openned
                size_t  mux_close_token;
                ppbox::mux::Muxer *muxer;           //保存open callbalk的指针   callback
                ppbox::demux::BufferDemuxer* demuxer;

                bool delay;
                typedef std::vector<MessageQType*>::iterator Iter;
                std::vector<MessageQType*> sessions; // session队列
            };
            
        private:
            util::daemon::Daemon & daemon_;

            boost::thread * dispatch_thread_;    //线程指针

            bool playing_;
            bool exit_;
            bool need_session_;  //不关联新的session
            bool playmode_;

            boost::uint32_t video_type_;          //视频索引值
            boost::uint32_t audio_type_;          //音频索引值

            framework::thread::MessageQueue<MessageQType*> * msgq_;  //消息队列
            //std::vector<Sink*> send_;
            Sink empty_sink_;
            std::vector<Sink*> sink_;
            Sink * default_sink_;
            session_callback_respone play_resq_;

protected:
            Movie* cur_mov_;  // 当前正在处理的影片
            Movie* append_mov_;  // 新的session插入到该队列
            //Movie mov_[2];
        };

    } // namespace rtsp
} // namespace ppbox

#endif // _PPBOX_RTSP_RTP_DISPATCHER_H_
