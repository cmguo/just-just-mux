// RawMuxer.h

#ifndef _PPBOX_MUX_RAW_RAW_MUXER_H_
#define _PPBOX_MUX_RAW_RAW_MUXER_H_

#include "ppbox/mux/MuxerBase.h"

namespace ppbox
{
    namespace mux
    {

        class RawMuxer
            : public MuxerBase
        {
        public:
            RawMuxer();

            virtual ~RawMuxer();

        private:
            virtual void do_open(
                MediaInfo & info);

            virtual void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);

            virtual void file_header(
                Sample & sample);

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & sample);

        private:
            std::string real_format_;
            boost::uint32_t time_scale_;
            boost::uint32_t video_time_scale_;
            boost::uint32_t audio_time_scale_;
        };

        PPBOX_REGISTER_MUXER("raw", RawMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RAW_RAW_MUXER_H_
