// RtspSession.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/tool/Dispatcher.h"
#include "ppbox/mux/MuxerModule.h"
#include "ppbox/mux/MuxError.h"

#include <ppbox/demux/DemuxerModule.h>
#include <ppbox/demux/base/BufferDemuxer.h>
#include <ppbox/demux/base/DemuxerError.h>
#include <ppbox/demux/base/SourceError.h>

#include <framework/system/LogicError.h>
#include <framework/thread/MessageQueue.h>
#include <framework/logger/LoggerStreamRecord.h>
#include <framework/logger/LoggerSection.h>
using namespace framework::thread;
using namespace framework::system::logic_error;
using namespace framework::logger;

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Dispatcher", 0)

namespace ppbox
{
    namespace mux
    {

        Dispatcher::Dispatcher(
            util::daemon::Daemon & daemon)
            : daemon_(daemon)
            , dispatch_thread_(NULL)
        {
            empty_sink_.attach();


            default_sink_ = &empty_sink_;
            default_sink_->attach();

            video_type_ = 0;
            audio_type_ = 0;
            playing_ = false;
            exit_ = false;
            playmode_ = true;

            msgq_ = new MessageQueue<MessageQType*>;

            cur_mov_= NULL;
            append_mov_ = NULL;


        }

        Dispatcher::~Dispatcher()
        {
            if (dispatch_thread_) 
            {
                dispatch_thread_->join();
                delete dispatch_thread_;
                dispatch_thread_ = NULL;
            }

            if (msgq_) 
            {
                delete msgq_;
                msgq_ = NULL;
            }
        }
       
        error_code Dispatcher::open(
            boost::uint32_t& session_id,
            std::string const & play_link,
            std::string const & format,
            bool need_session,
            session_callback_respone const & resp
            )
        {
            static size_t g_session_id = rand();
            session_id = g_session_id++;

            LOG_S(Logger::kLevelEvent, "[open] session_id:"<<session_id<<
                " playlink:"<<play_link<<" format:"<<format);

            msgq_->push(new MessageQType(PC_Open,session_id,resp,play_link,format,need_session));

            return error_code();
        }

        boost::system::error_code Dispatcher::setup(
            boost::uint32_t session_id, 
            size_t  index,
            Sink*  sink,
            session_callback_respone const & resp)
        {
            LOG_S(Logger::kLevelEvent, "[setup] session_id:"<<session_id<<" index:"<<index);
            msgq_->push(new MessageQType(PC_Setup,session_id,index,sink,resp));
            return error_code();
        }

        boost::system::error_code Dispatcher::setup(
            boost::uint32_t session_id, 
            Sink*  sink,
            session_callback_respone const & resp)
        {
            LOG_S(Logger::kLevelEvent, "[setup] session_id:"<<session_id);
            msgq_->push(new MessageQType(PC_Setup,session_id,-1,sink,resp));
            return error_code();
        }

        //only seek
        boost::system::error_code Dispatcher::seek(
            const boost::uint32_t session_id
            ,const boost::uint32_t begin
            ,const boost::uint32_t end
            ,session_callback_respone const & resp)
        {
            LOG_S(Logger::kLevelEvent, "[seek] session_id:"<<session_id<<" seek:"<<begin);
            msgq_->push(new MessageQType(PC_Seek,session_id,resp,begin,end));
            return error_code();
        }

        //seek + play
        boost::system::error_code Dispatcher::play(
            const boost::uint32_t session_id
            ,boost::uint32_t begin
            ,boost::uint32_t end
            ,session_callback_respone const &resp)
        {
            LOG_S(Logger::kLevelEvent, "[play_seek] session_id:"<<session_id);
            msgq_->push(new MessageQType(PC_Play,session_id,resp,begin,end));
            return boost::system::error_code();
        }

        //only play
        error_code Dispatcher::play(
            const boost::uint32_t session_id,
            session_callback_respone const & resp)
        {
            LOG_S(Logger::kLevelEvent, "[play] session_id:"<<session_id);
            msgq_->push(new MessageQType(PC_Play,session_id,resp));
            return error_code();
        }

        error_code Dispatcher::record(
            const boost::uint32_t session_id,
            session_callback_respone const & resp)
        {
            LOG_S(Logger::kLevelEvent, "[record] session_id:"<<session_id);
            msgq_->push(new MessageQType(PC_Record,session_id,resp));
            return error_code();
        }

        boost::system::error_code Dispatcher::resume(
            const boost::uint32_t session_id
            ,session_callback_respone const &resp)
        {
            LOG_S(Logger::kLevelEvent, "[resume] session_id:"<<session_id);
            msgq_->push(new MessageQType(PC_Resume,session_id,resp));
            return error_code();
        }

        error_code Dispatcher::pause(
            const boost::uint32_t session_id, 
            session_callback_respone const & resp)
        {
            LOG_S(Logger::kLevelEvent, "[pause] session_id:"<<session_id);
            msgq_->push(new MessageQType(PC_Pause,session_id,resp));

            return error_code();
        }

        error_code Dispatcher::close(
            const boost::uint32_t session_id)
        {
            LOG_S(Logger::kLevelEvent, "[close] session_id:"<<session_id);
            msgq_->push(new MessageQType(PC_Close,session_id)); 
            return error_code();
        }

        boost::system::error_code Dispatcher::start()
        {
            dispatch_thread_ = new boost::thread(
                boost::bind(&Dispatcher::thread_dispatch, this));
            return error_code();
        }

        boost::system::error_code Dispatcher::stop()
        {
            LOG_S(Logger::kLevelEvent, "[stop]");
            msgq_->push(new MessageQType(PC_Exit,0));
            dispatch_thread_->join();
            delete dispatch_thread_;
            dispatch_thread_ = NULL;
            return error_code();
        }

        ppbox::demux::DemuxerModule & Dispatcher::demuxer_module()
        {
            return util::daemon::use_module<demux::DemuxerModule>(daemon_);
        }

        MuxerModule & Dispatcher::muxer_module()
        {
            return util::daemon::use_module<mux::MuxerModule>(daemon_);
        }

        boost::system::error_code Dispatcher::thread_open(MessageQType* &param)
        {
            boost::system::error_code ec = boost::asio::error::would_block;

            if(param->play_link_.empty())
            {
                if(NULL == cur_mov_)
                {
                    ec = boost::asio::error::operation_aborted;
                    return ec;
                }
                else
                {
                    param->play_link_ = cur_mov_->play_link;
                    param->format_ = cur_mov_->format;
                }
            }

            if(NULL == append_mov_)
            {
                // closed
                //cur_mov_ = &mov_[0];
                cur_mov_ = new Movie();
                cur_mov_->Init(param);
                append_mov_ = cur_mov_;

                demuxer_module().async_open(
                    cur_mov_->play_link,
                    cur_mov_->close_token,
                    boost::bind(&Dispatcher::open_call_back_mux,this,cur_mov_->session_id, _1, _2));
            }
            else if (param->play_link_ != append_mov_->play_link || param->format_ != append_mov_->format)
            {
                boost::system::error_code ec1 = boost::asio::error::operation_aborted;
                // opening, opened, cancelling, cancel_delay, close_delay
                /*for (Movie::Iter iter = append_mov_->sessions.begin();
                    iter != append_mov_->sessions.end();
                    ++iter)
                {
                    //io_srv_.post(boost::bind((*iter)->resq_,ec));
                    (*iter)->resq_(ec1);
                    delete (*iter);
                }
                append_mov_->sessions.clear();*/
                clear_movie(append_mov_,ec1);
                clear_send(ec1);

                if (param->play_link_ != append_mov_->play_link) 
                {
                    // opening, opened, cancelling, cancel_delay, close_delay
                    if(append_mov_->close_token != 0) 
                    {
                        boost::system::error_code ec2;
                        // opening, opened, cancel_delay, close_delay
                        if (append_mov_->muxer)
                        {
                            // openned close_delay
                            muxer_module().close(append_mov_->mux_close_token,ec2);
                            //append_mov_->muxer->close();
                            append_mov_->mux_close_token = 0;
                            append_mov_->muxer = NULL;
                        }
                        demuxer_module().close(append_mov_->close_token,ec2);
                        append_mov_->close_token = 0;
                        // if opened or close_delay then append_mov_->demuxer = NULL;
                    }
                    if (append_mov_ == cur_mov_  )
                    { 
                        if(append_mov_->demuxer == NULL)
                        {
                            append_mov_ = new Movie();
                            append_mov_->Init(param);
                        }
                        else
                        {
                               append_mov_->demuxer = NULL;
                               demuxer_module().async_open(
                                   param->play_link_,
                                   append_mov_->close_token,
                                   boost::bind(&Dispatcher::open_call_back_mux,this,param->session_id_, _1, _2));
                        }
                    }
                    // cancelling, wait
                    append_mov_->muxer = NULL; // because 
                    append_mov_->play_link =  param->play_link_;
                }
                else //  if (param->play_link_ != append_mov_->play_link)
                {
                    // opening, opened, cancelling, cancel_delay, close_delay
                    if(append_mov_->muxer != NULL) 
                    {
                        //openned close_delay
                        // append_mov_->muxer->close();  //切换muxer
                        ec.clear();
                        append_mov_->muxer = muxer_module().open(
                            append_mov_->demuxer,
                            param->format_,
                            append_mov_->mux_close_token);
                    }
                }
            }
            else
            {//openning openned cancelling, cancel_delay, close_delay
                //play_link 相同 type也相同
                if (append_mov_->demuxer != NULL)
                {
                    // close_delay  openned
                    if (param->need_session) {
                        boost::uint32_t iseek = 0;
                        append_mov_->muxer->seek(iseek,ec);
                        append_mov_->muxer->reset();
                    }
                    ec.clear();
                }
            }
            // closed, opening, opened, cancelling, cancel_delay, close_delay
            
            append_mov_->format = param->format_;

            if (ec == boost::asio::error::would_block) 
            {
                append_mov_->delay = false;
                if (!param->need_session)
                {
                    append_mov_->sessions.insert(append_mov_->sessions.begin(),param);
                }
                else
                {
                    append_mov_->sessions.push_back(param);
                }
            } else if (ec) {
            } else {
                if (param->need_session) 
                {
                    append_mov_->session_id = param->session_id_;
                    clear_send(boost::asio::error::operation_aborted);
                    append_mov_->delay = false;
                }
            }

            return ec;
        }
        boost::system::error_code Dispatcher::thread_resume(MessageQType* &param)
        {
            boost::system::error_code ec;
            if (cur_mov_->delay ||  0  ==  cur_mov_->close_token ||cur_mov_->muxer ==NULL )
            {
                ec = ppbox::mux::error::mux_not_open;
            }
            else
            {
                playing_ = true;
            }
             return ec;
        }

        boost::system::error_code Dispatcher::thread_record(MessageQType* &param)
        {
            boost::system::error_code ec;
            if (cur_mov_->delay ||  0  ==  cur_mov_->close_token ||cur_mov_->muxer ==NULL )
            {
                ec = ppbox::mux::error::mux_not_open;
            }
            else
            {
                if(!play_resq_.empty())
                {
                    play_resq_(ec);
                }
                play_resq_.swap(param->resq_);
                playing_ = true;
                playmode_ = false;

            }

            return ec;
        }

        boost::system::error_code Dispatcher::thread_play(MessageQType* &param)
        {
            boost::system::error_code ec;
            if (cur_mov_->delay ||  0  ==  cur_mov_->close_token ||cur_mov_->muxer ==NULL )
            {
                ec = ppbox::mux::error::mux_not_open;
            }
            else
            {
                if(!play_resq_.empty())
                {
                   play_resq_(boost::asio::error::operation_aborted);
                   session_callback_respone().swap(play_resq_);
                }
                play_resq_.swap(param->resq_);
                playing_ = true;
                playmode_ = true;
            }

            return ec;
        }
        
        boost::system::error_code Dispatcher::thread_seek(MessageQType* &param)
        {
            boost::system::error_code ec;

            cur_mov_->seek = param->beg_;
            cur_mov_->muxer->seek(param->beg_,ec);

            size_t n = 0;
            while (ec == boost::asio::error::would_block) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(100));
                ++n;
                if (!msgq_->empty()) {
                    ec = boost::asio::error::operation_aborted;
                    break;
                }
                if (n == 5) {
                    cur_mov_->muxer->seek(param->beg_,ec);
                    n = 0;
                }
            }
			
            if (!ec)
            {  
				if((boost::uint32_t(-1) != param->end_))
				{
                    cur_mov_->muxer->reset();
                    ec.clear();
				}
				else
				{
					LOG_S(Logger::kLevelDebug,"[thread_seek] Not Send Head");
				}
            }
            else
            {
                LOG_S(Logger::kLevelError,"[thread_seek] Seek code : "<<ec.value()<<" msg"<<ec.message());
            }
            
            return ec;
        }

        boost::system::error_code Dispatcher::thread_pause(MessageQType* &param)
        {
            boost::system::error_code ec;
            playing_ = false;
            return error_code();
        }

        boost::system::error_code Dispatcher::thread_callback(MessageQType* &param)
        {
            boost::system::error_code ec;
            assert(NULL != cur_mov_);
            assert(NULL == cur_mov_->muxer);
            
            if(cur_mov_->close_token)
            {
                //openning cancel_delay
                cur_mov_->demuxer = param->demuxer;

                if (!cur_mov_->delay)
                {
                    //openning
                    cur_mov_->session_id = cur_mov_->sessions.back()->session_id_;

                    cur_mov_->muxer = muxer_module().open(cur_mov_->demuxer,
                        cur_mov_->format,
                        cur_mov_->mux_close_token);

                    if (NULL == cur_mov_->muxer)
                    {
                        //理论上open_mux不为NULL
                        cur_mov_->delay = true; //转close_delay
                    }
                    else
                    {
                        const ppbox::mux::MediaFileInfo & infoTemp = cur_mov_->muxer->mediainfo();
                        for (boost::uint32_t i = 0; i < infoTemp.stream_infos.size(); ++i) {
                            if (infoTemp.stream_infos[i].type == ppbox::demux::MEDIA_TYPE_VIDE) {
                                video_type_ = i;
                            } else if (infoTemp.stream_infos[i].type == ppbox::demux::MEDIA_TYPE_AUDI){
                                audio_type_ = i;
                            }
                        }
                    }

                    clear_movie(cur_mov_,param->ec);
                }
            }

            if (cur_mov_->close_token == 0 || param->ec)  // param->ec异常 cur_mov_->close_token 是否o处理一致
            { //canceling !ec
                if(cur_mov_->close_token)
                    demuxer_module().close(cur_mov_->close_token,ec);
                delete cur_mov_;

                if (append_mov_ != cur_mov_)
                {
                    //打开新列表  
                    cur_mov_ = append_mov_;
                    //cur_mov_->Init(param);
                    demuxer_module().async_open(
                        cur_mov_->play_link,
                        cur_mov_->close_token,
                        boost::bind(&Dispatcher::open_call_back_mux,this,cur_mov_->session_id, _1, _2));
                }
                else
                {
                    cur_mov_ = NULL;
                    append_mov_ = NULL;
                }
            }

            return ec;
        }


        boost::system::error_code Dispatcher::thread_stop(MessageQType* &param)
        {
            playing_ = false;

            if(NULL != append_mov_)
            {
                clear_movie(append_mov_,boost::asio::error::operation_aborted);

                delete append_mov_;
                if(cur_mov_ != append_mov_) 
                {
                    delete cur_mov_;
                }
                append_mov_ = NULL;
                cur_mov_ = NULL;
            }

            clear_send(boost::asio::error::operation_aborted);

            exit_ = true;

            return error_code();

        }

        class FindBySession
        {
        public:
            FindBySession(boost::uint32_t session):session_(session){}
            ~FindBySession(){}
            bool operator()(const MessageQType* msg) const
            {
                return (msg->session_id_ == session_);
            }
        private:
            boost::uint32_t session_;
        };


        boost::system::error_code Dispatcher::thread_close(MessageQType* &param)
        {
            boost::uint32_t session_id =  param->session_id_;

            //cur_mov_ 中去找， 如果有值表示在 openning状态, 如果为1直接转cancel_delay >1则删掉这个，如果在最后一个，session_id提前
            //如果没有值  canceling cancel_delay close_delay openned
            if (NULL == append_mov_)
            {
                //assert(false);
                return framework::system::logic_error::item_not_exist;
            }
            
            if (NULL != cur_mov_->muxer)
            {
                if (session_id == cur_mov_->session_id)
                {//openned close_delay
                    cur_mov_->delay = true; 
                    clear_send(boost::asio::error::operation_aborted);
                }
            }
            else
            {//openning canceling cancel_delay  //wait
                Movie::Iter iter = find_if(append_mov_->sessions.begin(), append_mov_->sessions.end() ,FindBySession(session_id));
                if (iter != append_mov_->sessions.end())
                {//openning wait
                    
                    LOG_S(Logger::kLevelError, "[thread_close] find_if "<<session_id);
                    MessageQType *pMsg = (*iter);
                    append_mov_->sessions.erase(iter);
                    delete pMsg;

                    if (append_mov_->sessions.empty())
                    {
                        if (append_mov_ == cur_mov_)
                        {//openning转 cancel_delay
                            append_mov_->delay = true;
                        }
                        else
                        {//wait
                            delete append_mov_;
                            append_mov_ = cur_mov_;
                        }
                    }
                    else
                    {
                        //append_mov_->session_id = append_mov_->sessions[append_mov_->sessions.size()-1]->session_id_;
                    }
                    
                }/*end  (iter != append_mov_->sessions.end()) */
            }
            return error_code();
        }

        boost::system::error_code Dispatcher::thread_timeout()
        {
            
            boost::system::error_code ec;
            if(NULL == append_mov_)
                return error_code();

                //openning canceling cancel_delay colose_delay openned

            if(cur_mov_->delay)
            {//close_delay cancel_delay canceling
                if(NULL != cur_mov_->muxer)
                {
                    muxer_module().close(append_mov_->mux_close_token,ec);
                    cur_mov_->muxer = NULL;
                }
         
                if (cur_mov_->close_token) {
                    demuxer_module().close(cur_mov_->close_token,ec);
                    cur_mov_->close_token = 0;
                    cur_mov_->delay = false;
                }

                if (NULL != cur_mov_->demuxer) {
                    delete cur_mov_;
                    cur_mov_ = NULL;
                    append_mov_ = NULL;
                }
            }

            return error_code();
        }

        boost::system::error_code Dispatcher::thread_setup(MessageQType* &param)
        {
            //LOG_S(Logger::kLevelEvent, "[thread_setup]");

            boost::system::error_code ec;

            size_t  control = param->control;
            Sink* sink = param->sink;
            if((size_t)-1 == control)
            {
                default_sink_->detach();
                default_sink_ = sink;
                default_sink_->attach();
            }
            else
            {
                sink->attach();
                if (control < sink_.size()) 
                {
                    if(!sink_[control]->detach())
                    {
                        if(!play_resq_.empty())
                        {
                            play_resq_(ec);
                            play_resq_.clear();
                        }
                    }
                    sink_[control] = sink;
                } else  if(control == sink_.size()) 
                {
                    sink_.push_back(sink);
                } else 
                {
                    empty_sink_.attach(control - sink_.size());
                    sink_.resize(control, &empty_sink_);
                    sink_.push_back(sink);
                }
                
            }
            return ec;
        }


        //----------------------------内部处理函数

        boost::system::error_code Dispatcher::thread_command(MessageQType* pMsgType)
        {
            boost::uint32_t session_id = 0;

            assert(NULL != pMsgType);
            session_id = pMsgType->session_id_;

            if ( (pMsgType->msg_ > PC_Session) && (NULL != cur_mov_ &&session_id !=0 && cur_mov_->muxer != NULL && session_id != cur_mov_->session_id) )
            {
                boost::system::error_code ec1 = ppbox::mux::error::mux_not_open;

                LOG_S(Logger::kLevelError, "[thread_command] session_id:"<<session_id<<
                    " old_id:"<<cur_mov_->session_id);

                if (!pMsgType->resq_.empty()) {
                    pMsgType->resq_(ec1);
                }
                delete pMsgType;

                //过滤掉不必要的处理
                if (playing_)
                    play();

                return boost::system::error_code();
            }

            if( 0 == session_id && pMsgType->msg_ > PC_Session)
            {
                if(NULL != cur_mov_)
                {
                    pMsgType->session_id_ = cur_mov_->session_id;
                }
                else
                {
                    boost::system::error_code ec1 =  ppbox::mux::error::mux_not_open;
                    if (!pMsgType->resq_.empty()) {
                        pMsgType->resq_(ec1);
                    }
                    delete pMsgType;
                    //assert(false);
                    //过滤掉不必要的处理
                    if (playing_)
                        play();
                    return ec1;
                }
            }

            boost::system::error_code ec;
            switch (pMsgType->msg_)
            {
            case PC_Open:
                {
                    ec = thread_open(pMsgType);
                }
                break;
            case PC_Play:
                {
                    ec = thread_play(pMsgType);
                }
                break;
            case PC_Seek:
                {
                    ec = thread_seek(pMsgType);
                }
                break;
            case PC_Pause:
                {
                    ec = thread_pause(pMsgType);
                }
                break;
            case PC_Close:
                {
                    ec = thread_close(pMsgType);
                }
                break;
            case PC_Callback:
                {
                    ec = thread_callback(pMsgType);
                }
                break;  
            case PC_Exit:
                {
                    ec = thread_stop(pMsgType);
                }
                break;
            case PC_Setup:
                {
                    ec = thread_setup(pMsgType);
                }
                break;
            case PC_Resume:
                {
                    ec = thread_resume(pMsgType);
                }
                break;
            case PC_Record:
                {
                    ec = thread_record(pMsgType);
                }
                break;
            default:
                //异常情况
                LOG_S(Logger::kLevelError,"[thread_dispatch][Unknow Msg Type]");
                assert(false);
                break;
            }

            if (ec != boost::asio::error::would_block) {
                if (!pMsgType->resq_.empty()) {
                    pMsgType->resq_(ec);
                }
                delete pMsgType;
            }

            if (playing_)
                play();
            
            return boost::system::error_code();
        }

        void Dispatcher::open_call_back_mux(
            const boost::uint32_t session_id,
            boost::system::error_code const & ec,
            ppbox::demux::BufferDemuxer * muxer)
        {
            msgq_->push(new MessageQType(PC_Callback,ec,muxer));
            return;
        }

        static const std::string status_str[8] = {
            "canceling_1", 
            "opening", 
            "closed_1", 
            "openned", 
            "canceling_2", 
            "cancel_delay", 
            "closed_2", 
            "close_delay", 
        };
        static const std::string play_event_str[11] = {
            "PC_Open", 
            "PC_Close", 
            "PC_Callback", 
            "PC_Exit", 
            "PC_Session", 
            "PC_Setup", 
            "PC_Record", 
            "PC_Play",
            "PC_Resume",
            "PC_Seek",
            "PC_Pause",
        };
        static const std::string status_str_empty("empty");

        std::string const & Dispatcher::status_string()
        {
            if (cur_mov_ == NULL)
            {
                return status_str_empty;
            }
            size_t index = 0;
            if (cur_mov_->close_token)
                index |= 1;
            if (cur_mov_->demuxer)
                index |= 2;
            if (cur_mov_->delay)
                index |= 4;
            return status_str[index];
        }

        void Dispatcher::thread_dispatch()
        {
            MessageQType* pMsgType = NULL;

            while(!exit_) {
                if( msgq_->timed_pop(pMsgType,boost::posix_time::milliseconds(10*1000)) )
                {
                    LOG_SECTION();

                    boost::uint32_t session_id = pMsgType->session_id_;
                    PlayControl msg = pMsgType->msg_;
                    LOG_S(Logger::kLevelDebug,"[thread_dispatch] begin, session:" << session_id 
                        << ", msg:" << play_event_str[msg]
                        << ", status:" << status_string());

                    thread_command(pMsgType);

                    LOG_S(Logger::kLevelDebug,"[thread_dispatch] ended, session:" << session_id 
                        << ", msg:" << play_event_str[msg]
                        << ", status:" << status_string());
                }
                else if (cur_mov_ && cur_mov_->delay)
                {
                    //超时处理位置
                    LOG_SECTION();
                    LOG_S(Logger::kLevelDebug,"[thread_dispatch] begin, session:0, msg:PC_Timeout, status:" << status_string());

                    thread_timeout();

                    LOG_S(Logger::kLevelDebug,"[thread_dispatch] ended, session:0, msg:PC_Timeout, status:" << status_string());
                }
                else
                {
                    std::cout << "[thread_dispatch] idle" << std::endl;
                }
            }

        }
        void Dispatcher::clear_send(boost::system::error_code const & ec)
        {
            playing_ = false;
            //std::vector<Sink*> sink_;
            std::vector<Sink*>::iterator iter = sink_.begin();
            for(; iter != sink_.end(); ++iter)
            {
                (*iter)->detach();
            }
            sink_.clear();

            default_sink_->detach();
            default_sink_ = &empty_sink_;
            default_sink_->attach();

            if(!play_resq_.empty())
            {
                LOG_S(Logger::kLevelEvent, "[detach play_resq] ");
                play_resq_(ec);
                play_resq_.clear();
            }
        }

        void Dispatcher::clear_movie(Movie* movie,boost::system::error_code const & ec)
        {
            MessageQType* pp =NULL;
            for (size_t ii = 0; ii < movie->sessions.size(); ++ii )
            {
                pp = movie->sessions[ii];
                pp->resq_(!pp->need_session || ec || ii + 1 == movie->sessions.size() ? ec : boost::asio::error::operation_aborted);
                delete pp;
            }
            movie->sessions.clear();
        }

        boost::system::error_code Dispatcher::play()
        {
            LOG_S(Logger::kLevelEvent, "[internal_play] session:" << cur_mov_->session_id);

            bool start_time_valid = false;
            //playing_ = true;

            boost::uint64_t seek_end64 = -1;
            boost::posix_time::ptime start_time;
            boost::system::error_code  ec;

            while (true) 
            {
                if (!msgq_->empty()) {
                    LOG_S(Logger::kLevelEvent, "[internal_play] new message arrived");
                    break;
                }

                ec.clear();
                ppbox::demux::Sample  tag ;
                cur_mov_->muxer->read(tag,ec);

                if (!ec)
                {
                    //限制播放的最大时长
                    if (tag.ustime > seek_end64)
                    {
                        //if(!cur_mov_->play_resq.empty())
                        {
                            LOG_S(Logger::kLevelEvent, "[internal_play] play time end");
                            play_resq_(ec);
                            play_resq_.clear();
                        }
                        playing_ = false;
                        break;
                    }

                    if (!start_time_valid && tag.itrack != boost::uint32_t(-1)) {
                        start_time = 
                            boost::posix_time::microsec_clock::universal_time()
                            - boost::posix_time::microseconds(tag.ustime);
                        start_time_valid = true;
                    }
                    boost::posix_time::ptime now = 
                        boost::posix_time::microsec_clock::universal_time();
                    boost::posix_time::ptime send_time = 
                        start_time + boost::posix_time::microseconds(tag.ustime);

                    //限速模块
                    if(playmode_  && tag.itrack == audio_type_)
                    {
                        if (send_time > now)
                        {
                            //std::cout<<"Sleep :"<< (send_time - now).total_microseconds() << std::endl;
                            boost::this_thread::sleep(send_time - now);
                        }
                    }
                    
                    //输出音视频信息
                    if(tag.itrack < sink_.size())
                    {
                        ec = sink_[tag.itrack]->write(send_time, tag);
                    }
                    else
                    {
                        ec = default_sink_->write(send_time, tag);
                    }
                }
                else
                {
                    if(ec != boost::asio::error::would_block )
                    {
                        if (sink_.size() > 0)
                        {
                            for(size_t ii = 0; ii < sink_.size(); ++ii)
                            {
                                sink_[ii]->on_finish(ec);
                            }
                        }
                        else
                        {
                            default_sink_->on_finish(ec);
                        }
                    }
                }

                if(ec)
                {
                    if(ec != boost::asio::error::would_block )
                    {
                        LOG_S(Logger::kLevelError, "[internal_play] on_error, ec: "<<ec.message());

                        playing_ = false;
                        play_resq_(ec);
                        play_resq_.clear();
                        break;
                    }
                    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
                    start_time_valid = false;
                    
                }
            }

            LOG_S(Logger::kLevelEvent, "[internal_play] exiting...");
            //playing_ = false;
            return ec;
        }

        boost::system::error_code Dispatcher::get_info(
            ppbox::mux::MediaFileInfo & info)
        {
            boost::system::error_code ec;
            if(NULL != cur_mov_ &&  NULL != cur_mov_->muxer)
            {
                info = cur_mov_->muxer->mediainfo();
            }
            else
            {
                ec = ppbox::mux::error::mux_not_open;
            }
            return ec;
        }

    } // namespace rtspd
} // namespace ppbox
