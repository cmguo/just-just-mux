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
            PC_Open,  //������
            PC_Close,  //  �رղ���,�ص���������
            PC_Callback,  //��״̬,�ص����������̴߳���
            PC_Exit,   //�߳��˳�
            PC_Session = 8, 
            PC_Setup,  //�������Ĵ���
            PC_Play,   // ����
            PC_Resume, //�ָ�����s
            PC_Seek,  //  �������϶���
            PC_Pause,   //��ͣ
        };

/*#ifdef AAA
        enum ThreadStatus
        {
            TS_Closed,       //���У���Ŀ�ر�
            TS_Openning,  //���ڴ�
            TS_Canceling,    //�Ȼص����ֱ�ӹر�
            TS_Cancel_Delay,  //�Ȼص���תTS_Close_Delay״̬
            TS_Working,   //������
            TS_Pause,    //��ͣ����
            TS_Close_Delay, //�ӳٹر�
            TS_Last_Callback,  //�ص���͹ر��߳�
            TS_Exit,     //�̹߳ر�
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
