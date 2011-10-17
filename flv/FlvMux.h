// FlvMux.h
#ifndef   _PPBOX_MUX_FLV_FLVMUX_H_
#define   _PPBOX_MUX_FLV_FLVMUX_H_

#include "ppbox/mux/MuxerBase.h"
#include <ppbox/demux/DemuxerBase.h>

namespace ppbox
{
    namespace mux
    {
        class FlvMuxWriter;
        class FlvMux
            : public MuxerBase
        {

        public:
            enum StatusEnum
            {
                closed, 
                opened, 
            };

        public:
            FlvMux();
            ~FlvMux();

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

        private:
            void create_metadata(void);

        private:
            StatusEnum       state_;
            FlvMuxWriter *   flv_writer_;
            ppbox::demux::Sample sample_; // 当前读取的数据帧
            bool paused_;
            //bool need_seek_time_;
            bool is_read_head_;
            bool is_read_metadata_;
        };
    }
}

#endif // End _PPBOX_MUX_FLV_FLVMUX_H_
