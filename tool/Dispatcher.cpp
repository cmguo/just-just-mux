// RtspSession.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/tool/Dispatcher.h"
#include "ppbox/mux/MuxerModule.h"
#include "ppbox/mux/MuxError.h"

#include <ppbox/demux/DemuxerModule.h>
#include <ppbox/demux/DemuxerError.h>

#include <framework/system/LogicError.h>
#include <framework/thread/MessageQueue.h>
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
            : demuxer_module_(util::daemon::use_module<demux::DemuxerModule>(daemon))
            , muxer_module_(util::daemon::use_module<MuxerModule>(daemon))
            , dispatch_thread_(NULL)
        {
            empty_sink_.attach();

            default_sink_ = NULL;

            video_type_ = 0;
            audio_type_ = 0;
            playing_ = false;
            exit_ = false;
            msgq_ = new MessageQueue<MessageQType*>;

            cur_mov_= NULL;
            append_mov_ = NULL;

            dispatch_thread_ = new boost::thread(
                boost::bind(&Dispatcher::thread_dispatch, this));
        }

        Dispatcher::~Dispatcher()
        {
            if (dispatch_thread_) 
            {
                stop();
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

            LOG_S(Logger::kLevelEvent, "[open] "<<"["<<session_id<<"]"
                <<"play:"<<play_link<<" format:"<<format);

            msgq_->push(new MessageQType(PC_Open,session_id,resp,play_link,format,need_session));

            return error_code();
        }

        boost::system::error_code Dispatcher::setup(
            boost::uint32_t session_id, 
            size_t  index,
            Sink*  sink,
            session_callback_respone const & resp)
        {
            LOG_S(Logger::kLevelEvent, "[setup] "<<"["<<session_id<<"] index:"<<index);
            msgq_->push(new MessageQType(PC_Setup,session_id,index,sink,resp));
            return error_code();
        }

        boost::system::error_code Dispatcher::setup(
            boost::uint32_t session_id, 
            Sink*  sink,
            session_callback_respone const & resp)
        {
            LOG_S(Logger::kLevelEvent, "[setup] "<<"["<<session_id);
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
            LOG_S(Logger::kLevelEvent, "[seek] "<<"["<<session_id<<"] seek:"<<begin);
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
            LOG_S(Logger::kLevelEvent, "[play_seek] "<<"["<<session_id<<"]");
            msgq_->push(new MessageQType(PC_Play,session_id,resp,begin,end));
            return boost::system::error_code();
        }

        //only play
        error_code Dispatcher::play(
            const boost::uint32_t session_id,
            session_callback_respone const & resp)
        {
            LOG_S(Logger::kLevelEvent, "[play] "<<"["<<session_id<<"]");
            msgq_->push(new MessageQType(PC_Play,session_id,resp));
            return error_code();
        }

        boost::system::error_code Dispatcher::resume(
            const boost::uint32_t session_id
            ,session_callback_respone const &resp)
        {
            LOG_S(Logger::kLevelEvent, "[play] "<<"["<<session_id<<"]");
            msgq_->push(new MessageQType(PC_Resume,session_id,resp));
            return error_code();
        }

        error_code Dispatcher::pause(const boost::uint32_t session_id,session_callback_respone const & resp)
        {
            LOG_S(Logger::kLevelEvent, "[pause] "<<"["<<session_id<<"]");
            msgq_->push(new MessageQType(PC_Pause,session_id,resp));

            return error_code();
        }

        error_code Dispatcher::close(const boost::uint32_t session_id)
        {
            LOG_S(Logger::kLevelEvent, "[close] "<<"["<<session_id<<"]");
            msgq_->push(new MessageQType(PC_Close,session_id)); 
            return error_code();
        }

        boost::system::error_code Dispatcher::stop()
        {
            LOG_S(Logger::kLevelEvent, "[stop]");
            session_callback_respone resp;
            msgq_->push(new MessageQType(PC_Exit,0));
            dispatch_thread_->join();
            dispatch_thread_ = NULL;
            return error_code();
        }

        boost::system::error_code Dispatcher::thread_open(MessageQType* &param)
        {
            //非三种状态不处理，绝对性的异常
            LOG_S(Logger::kLevelEvent, "[thread_open] ["<<(boost::uint32_t)this<<"]["<<param->session_id_<<"]");

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

                demuxer_module_.async_open(
                    cur_mov_->play_link,
                    cur_mov_->close_token,
                    boost::bind(&Dispatcher::open_call_back_mux,this,cur_mov_->session_id, _1, _2));
            }
            else if (param->play_link_ != append_mov_->play_link || param->format_ != append_mov_->format)
            {
                boost::system::error_code ec1 = boost::asio::error::operation_aborted;
                // opening, opened, cancelling, cancel_delay, close_delay
                for (Movie::Iter iter = append_mov_->sessions.begin();
                    iter != append_mov_->sessions.end();
                    ++iter)
                {
                    //io_srv_.post(boost::bind((*iter)->resq_,ec));
                    (*iter)->resq_(ec1);
                    delete (*iter);
                }
                append_mov_->sessions.clear();// 回调
                clear_send();

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
                            muxer_module_.close(append_mov_->mux_close_token,ec2);
                            //append_mov_->muxer->close();
                            append_mov_->mux_close_token = 0;
                            append_mov_->muxer = NULL;
                        }
                        demuxer_module_.close(append_mov_->close_token,ec2);
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
                               demuxer_module_.async_open(
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
                        append_mov_->muxer = muxer_module_.open(
                            append_mov_->demuxer,
                            param->format_,
                            append_mov_->mux_close_token);
                    }
                }
            }
            else
            {//openning openned cancelling, cancel_delay, close_delay
                //play_link 相同 type也相同
                if (append_mov_->demuxer != NULL && param->need_session)
                {
                    // close_delay  openned
                    boost::uint32_t iseek = 0;
                    append_mov_->muxer->seek(iseek,ec);
                    append_mov_->muxer->reset();
                }
                ec.clear();
            }
            // closed, opening, opened, cancelling, cancel_delay, close_delay
            
            append_mov_->format = param->format_;

            if (ec == boost::asio::error::would_block) {
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
                if (param->need_session) {
                    append_mov_->session_id = param->session_id_;
                    clear_send();
                    append_mov_->delay = false;
                }
            }

            return ec;
        }
        boost::system::error_code Dispatcher::thread_resume(MessageQType* &param)
        {
            LOG_S(Logger::kLevelEvent, "[thread_resume] ["<<(boost::uint32_t)this<<"]["<<cur_mov_->session_id<<"]");
            boost::system::error_code ec;

            if (cur_mov_->delay ||  0  ==  cur_mov_->close_token ||cur_mov_->muxer ==NULL )
            {
                ec = ppbox::mux::error::mux_not_open;
                LOG_S(Logger::kLevelEvent, "[thread_resume] wrong enter here");
            }
            else
            {
                playing_ = true;
            }
             return ec;
        }

        boost::system::error_code Dispatcher::thread_play(MessageQType* &param)
        {
            LOG_S(Logger::kLevelEvent, "[thread_play] ["<<(boost::uint32_t)this<<"]["<<cur_mov_->session_id<<"]");
            boost::system::error_code ec;
            //assert(cur_mov_->play_resq.empty());
            if (cur_mov_->delay ||  0  ==  cur_mov_->close_token ||cur_mov_->muxer ==NULL )
            {
                ec = ppbox::mux::error::mux_not_open;
                LOG_S(Logger::kLevelEvent, "[thread_play] wrong enter here");
            }
            else
            {
                if(!cur_mov_->play_resq.empty())
                {
                    cur_mov_->play_resq(ec);
                }
                cur_mov_->play_resq.swap(param->resq_);
                playing_ = true;
            }

            return ec;
        }
        
        boost::system::error_code Dispatcher::thread_seek(MessageQType* &param)
        {
            //ENTER_(thread_seek);

            boost::system::error_code ec;
            LOG_S(Logger::kLevelEvent, "[thread_seek] ["<<(boost::uint32_t)this<<"]["<<cur_mov_->session_id<<"]"
                <<"[PC_Seek] beg:"<<param->beg_);

            cur_mov_->seek = param->beg_;
            
            cur_mov_->muxer->seek(param->beg_,ec);

            if(!ec || ec == boost::asio::error::would_block)
            {
                cur_mov_->muxer->reset();
                ec.clear();
            }
            else
            {
                LOG_S(Logger::kLevelError,"[thread_seek] Seek code : "<<ec.value()<<" msg"<<ec.message().c_str());
            }
            
            return ec;
        }

        boost::system::error_code Dispatcher::thread_pause(MessageQType* &param)
        {
            boost::system::error_code ec;
            LOG_S(Logger::kLevelEvent, "[thread_pause] ["<<(boost::uint32_t)this<<"]["<<cur_mov_->session_id<<"][PC_Pause] ");
            playing_ = false;
            return error_code();
        }

        boost::system::error_code Dispatcher::thread_callback(MessageQType* &param)
        {
            boost::system::error_code ec;
            LOG_S(Logger::kLevelEvent, "[thread_callback] ["<<(boost::uint32_t)this<<"]["<<cur_mov_->session_id<<"]");

            //openning canning cancel_delay

            assert(NULL != cur_mov_);
            assert(NULL == cur_mov_->muxer);

            

            if (cur_mov_->close_token == 0 || param->ec)  // param->ec异常 cur_mov_->close_token 是否o处理一致
            { //canceling !ec
                demuxer_module_.close(cur_mov_->close_token,ec);
                cur_mov_->clear();
                delete cur_mov_;

                if (append_mov_ != cur_mov_)
                {
                    //打开新列表  
                    cur_mov_ = append_mov_;
                    //cur_mov_->Init(param);
                    demuxer_module_.async_open(
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
            else
            { 
                //openning cancel_delay
                cur_mov_->demuxer = param->demuxer;
                if (!cur_mov_->delay)
                {//openning
                    //cur_mov_->muxer = open(cur_mov_->demuxer,ec); //获取muxer
                    cur_mov_->muxer = muxer_module_.open(cur_mov_->demuxer,
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
                        video_type_ = infoTemp.video_index;
                        audio_type_ = infoTemp.audio_index;

                        cur_mov_->session_id = cur_mov_->sessions.back()->session_id_;
                    }

                }
            }
            if(NULL != cur_mov_ && cur_mov_->sessions.size() > 0)   //防止是 cancel_delay状态
            {
                MessageQType* pp =NULL;
                for (size_t ii = 0; ii < cur_mov_->sessions.size()-1; ++ii )
                {
                    pp = cur_mov_->sessions[ii];
                    pp->resq_(pp->need_session ? boost::asio::error::operation_aborted : param->ec);
                    delete pp;
                }
                pp = cur_mov_->sessions.back();
                pp->resq_(param->ec);
                delete pp;
                cur_mov_->sessions.clear();
            }
            return ec;
        }


        boost::system::error_code Dispatcher::thread_stop(MessageQType* &param)
        {
            if (NULL == append_mov_)
            {
                assert(false);
                return error_code();
            }
            LOG_S(Logger::kLevelEvent, "[thread_stop] ["<<(boost::uint32_t)this<<"]["<<cur_mov_->session_id<<"]");

            exit_ = true;
            playing_ = false;
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

            LOG_S(Logger::kLevelEvent, "[thread_close] ["<<(boost::uint32_t)this<<"]["<<session_id<<"]");
            //cur_mov_ 中去找， 如果有值表示在 openning状态, 如果为1直接转cancel_delay >1则删掉这个，如果在最后一个，session_id提前
            //如果没有值  canceling cancel_delay close_delay openned
            if (NULL == append_mov_)
            {
                LOG_S(Logger::kLevelError, "[thread_close] NULL == append_mov_");
                //assert(false);
                return framework::system::logic_error::item_not_exist;
            }
            
            if (NULL != cur_mov_->muxer)
            {
                if (session_id == cur_mov_->session_id)
                {//openned close_delay
                    cur_mov_->delay = true; 
                    clear_send();
                }
            }
            else
            {//openning canceling cancel_delay  //wait
                Movie::Iter iter = find_if(append_mov_->sessions.begin(), append_mov_->sessions.end() ,FindBySession(session_id));
                if (iter != append_mov_->sessions.end())
                {//openning wait
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
                LOG_S(Logger::kLevelEvent, "[thread_timeout] ");

                if(NULL != cur_mov_->muxer)
                {
                    muxer_module_.close(append_mov_->mux_close_token,ec);
                    cur_mov_->muxer = NULL;
                }
         
                if (cur_mov_->close_token) {
                    demuxer_module_.close(cur_mov_->close_token,ec);
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
            LOG_S(Logger::kLevelEvent, "[thread_setup]");
            boost::system::error_code ec;
            size_t  control = param->control;
            Sink* sink = param->sink;
            if((size_t)-1 == control)
            {
                clear_send();

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
                        if(NULL != cur_mov_ && !cur_mov_->play_resq.empty())
                        {
                            LOG_S(Logger::kLevelEvent, "[detach play_resq] ");
                            cur_mov_->play_resq(ec);
                            cur_mov_->play_resq.clear();
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

                LOG_S(Logger::kLevelError, "[thread_command] ["<<(boost::uint32_t)this<<"]["<<session_id<<"]"
                    <<"Old_ID:"<<cur_mov_->session_id<<" [MsgType] "<<pMsgType->msg_);

                if (!pMsgType->resq_.empty()) {
                    pMsgType->resq_(ec1);
                }
                delete pMsgType;
                //assert(false);
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
            ppbox::demux::Demuxer * muxer)
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
                if( msgq_->timed_pop(pMsgType,boost::posix_time::milliseconds(10000)) )
                {
                    LOG_S(Logger::kLevelDebug,"[thread_dispatch] begin status:" << status_string());
                    thread_command(pMsgType);                
                    LOG_S(Logger::kLevelDebug,"[thread_dispatch] end status:" << status_string());
                }
                else
                {
                    //超时处理位置
                    thread_timeout();
                }
            }

        }
        void Dispatcher::clear_send()
        {
            playing_ = false;
            //std::vector<Sink*> sink_;
            boost::system::error_code ec;
            std::vector<Sink*>::iterator iter = sink_.begin();
            for(; iter != sink_.end(); ++iter)
            {
                (*iter)->detach();
            }
            if (NULL != default_sink_)
            {
                default_sink_->detach();
                default_sink_ = NULL;

            }
            if(!cur_mov_->play_resq.empty())
            {
                LOG_S(Logger::kLevelEvent, "[detach play_resq] ");
                cur_mov_->play_resq(ec);
                cur_mov_->play_resq.clear();
            }
            sink_.clear();
        }

        boost::system::error_code Dispatcher::play()
        {
            LOG_S(Logger::kLevelEvent, "[play] ["<<(boost::uint32_t)this<<"]["<<cur_mov_->session_id<<"]");

            bool start_time_valid = false;
            //playing_ = true;

            boost::uint64_t seek_end64 = -1;
            boost::posix_time::ptime start_time;
            boost::system::error_code  ec;

            while (msgq_->empty())
            {
                ec.clear();
                ppbox::demux::Sample  tag ;
                cur_mov_->muxer->read(tag,ec);

                if (!ec)
                {
                    if(tag.itrack == audio_type_)
                    {
                        if (!start_time_valid) {
                            start_time = 
                                boost::posix_time::microsec_clock::universal_time()
                                - boost::posix_time::microseconds(tag.ustime);
                            start_time_valid = true;
                        }
                        if (tag.ustime > seek_end64)
                        {
                            //if(!cur_mov_->play_resq.empty())
                            {
                                LOG_S(Logger::kLevelEvent, "[play end] play_resq");
                                cur_mov_->play_resq(ec);
                                cur_mov_->play_resq.clear();
                            }
                            playing_ = false;
                            break;
                        }
                        boost::posix_time::ptime now = 
                            boost::posix_time::microsec_clock::universal_time();
                        boost::uint64_t now_time = 
                            (now - start_time).total_microseconds();

                        if (tag.ustime > now_time)

                            boost::this_thread::sleep(boost::posix_time::microseconds(tag.ustime - now_time));
                    }
                    
                    if (NULL != default_sink_)
                    {
                        ec = default_sink_->write(tag);
                    }
                    else if(tag.itrack < sink_.size())
                    {
                        ec = sink_[tag.itrack]->write(tag);
                    }
                    else
                    {
                        LOG_S(Logger::kLevelError, "[play] ERROR tag.itrack is Big:"<<tag.itrack);
                    }
                }

                if(ec)
                {
                    
                    if(ec != boost::asio::error::would_block )
                    {
                        LOG_S(Logger::kLevelError, "[play] ["<<(boost::uint32_t)this<<" ec:"
                            <<ec<<" msg"<<ec.message().c_str());
                        playing_ = false;
                    
                        //if(!cur_mov_->play_resq.empty())
                        {
                            LOG_S(Logger::kLevelEvent, "[play end] play_resq");
                            cur_mov_->play_resq(ec);
                            cur_mov_->play_resq.clear();
                        }

                        break;
                    }
                    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
                    
                }
            }

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
