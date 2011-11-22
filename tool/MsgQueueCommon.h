#ifndef _PPBOX_MUX_MSGQ_COMMON_H_
#define _PPBOX_MUX_MSGQ_COMMON_H_

#include <string>


namespace ppbox
{
    namespace demux
    {
        class PptvDemuxer;
    }

    namespace mux
    {
        class Sink;

        enum PlayControl
        {
            PC_Open,  //打开链接
            PC_Close,  //  关闭播放,回调回来后复用
            PC_Callback,  //打开状态,回调函数交给线程处理
            PC_Exit,   //线程退出
            PC_Session = 8, 
            PC_Setup,  //用于流的创建
            PC_Play,   // 播放
            PC_Resume, //恢复播放s
            PC_Seek,  //  进度条拖动到
            PC_Pause,   //暂停
        };

/*#ifdef AAA
        enum ThreadStatus
        {
            TS_Closed,       //空闲，节目关闭
            TS_Openning,  //正在打开
            TS_Canceling,    //等回调后就直接关闭
            TS_Cancel_Delay,  //等回调后转TS_Close_Delay状态
            TS_Working,   //工作中
            TS_Pause,    //暂停工作
            TS_Close_Delay, //延迟关闭
            TS_Last_Callback,  //回调后就关闭线程
            TS_Exit,     //线程关闭
            TS_EXPTION
        };
#endif*/



        typedef boost::function<void (
            boost::system::error_code const &)
        >  session_callback_respone;

        struct MessageQType
        {
            MessageQType(){}
            //for common
            MessageQType(
                PlayControl msg , 
                boost::uint32_t session_id,
                const session_callback_respone& resq)
                :msg_(msg),
                session_id_(session_id),
                resq_(resq)
            {
            }

            //For seek
            MessageQType(
                PlayControl msg,
                boost::uint32_t session_id,
                const session_callback_respone& resq,
                boost::uint32_t beg,
                boost::uint32_t end)
                :msg_(msg),
                session_id_(session_id),
                resq_(resq),
                beg_(beg),
                end_(end)
                
            {
            }
            MessageQType(
                PlayControl msg,
                boost::uint32_t session_id)
                :msg_(msg),
                session_id_(session_id)

            {
            }
            MessageQType(
                PlayControl msg,
                boost::system::error_code ec,
                ppbox::demux::PptvDemuxer* demuxer)
                :msg_(msg),
                ec(ec),
                demuxer(demuxer)

            {
            }
            MessageQType(
                PlayControl msg ,
                boost::uint32_t session_id,
                const session_callback_respone& resq,
                std::string play_link,
                std::string format,
                bool need_session)
                :msg_(msg),
                session_id_(session_id),
                resq_(resq),
                play_link_(play_link),
                format_(format),
                need_session(need_session)
            {
            }

            MessageQType(
                PlayControl msg ,
                boost::uint32_t session_id,
                size_t control,
                Sink* sink,
                const session_callback_respone& resq)
                :msg_(msg),
                session_id_(session_id),
                resq_(resq),
                control(control),
                sink(sink)
            {
            }


            PlayControl msg_;
            boost::uint32_t session_id_;
            session_callback_respone resq_;

            //Seek
            boost::uint32_t beg_;
            boost::uint32_t end_;

            //Open
            std::string play_link_;
            std::string format_;
            bool need_session;

            //Callback
            boost::system::error_code ec;
            ppbox::demux::PptvDemuxer* demuxer;

            //Setup
            size_t  control;
            Sink* sink;
        };

    }
}

#endif
