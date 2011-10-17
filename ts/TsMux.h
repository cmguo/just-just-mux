// TsMux.h

#ifndef _PPBOX_MUX_TS_TS_MUX_H_
#define _PPBOX_MUX_TS_TS_MUX_H_

#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/ts/Ap4Mpeg2Ts.h"

#include <bento4/Core/Ap4.h>

#include <deque>

namespace ppbox
{
    namespace mux
    {
        class TsMux
            : public MuxerBase
        {
        public:
            enum StatusEnum
            {
                closed, 
                opened, 
            };

        public:
            TsMux();
            ~TsMux();

        public:
            boost::system::error_code open(
                demux::Demuxer * demuxer, boost::system::error_code & ec);

            boost::system::error_code read(
                MuxTagEx * tag,
                boost::system::error_code & ec);

            boost::system::error_code seek(
                boost::uint32_t time,
                boost::system::error_code & ec);

            boost::system::error_code pause(
                boost::system::error_code & ec);

            boost::system::error_code resume(
                boost::system::error_code & ec);

            void close(void);

            void reset(void);

            ppbox::demux::Sample & get_sample(void);

            unsigned char const * get_head(boost::uint32_t & size);

            void set_ipad(bool is_ipad);

        private:
            boost::system::error_code get_sort_sample(
                ppbox::demux::Sample& sample,
                boost::system::error_code & ec);

            boost::system::error_code get_ignored_sample(
                ppbox::demux::Sample& sample,
                boost::system::error_code & ec);

        private:
            StatusEnum       state_;
            ppbox::demux::Sample sample_;
            bool paused_;
            bool need_seek_time_;
            bool is_read_head_;
            bool ipad_;
            bool is_wait_sync_;
            // Buffer
            AP4_MemoryByteStream * pmt_;
            // ����֡��Ӧ��ts��
            AP4_MemoryByteStream * ts_sample_;
            // TS Handle
            AP4_Mpeg2TsWriter ts_writer_;
            AP4_Mpeg2TsWriter::SampleStream* audio_stream_;
            AP4_Mpeg2TsWriter::SampleStream* video_stream_;
            // avc config, sps, pps
            AvcConfig *avc_config_;
            std::deque<ppbox::demux::Sample> queue_sample_[2];
        };
    }
}

#endif // _PPBOX_MUX_TS_TS_MUX_H_