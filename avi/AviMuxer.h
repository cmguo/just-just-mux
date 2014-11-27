// AviMuxer.h

#ifndef _JUST_MUX_AVI_AVI_MUXER_H_
#define _JUST_MUX_AVI_AVI_MUXER_H_

#include "just/mux/Muxer.h"
#include "just/mux/avi/AviDataContext.h"

#include <just/avformat/avi/lib/AviFile.h>

#include <util/buffers/StreamBuffer.h>

namespace just
{
    namespace mux
    {

        class AviTransfer;

        class AviMuxer
            : public Muxer
        {
        public:
            AviMuxer(
                boost::asio::io_service & io_svc);

            ~AviMuxer();

        protected:
            virtual void do_open(
                MediaInfo & info);

            virtual void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);

            virtual void file_header(
                Sample & sample);

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & sample);

            virtual void file_tail(
                Sample & sample);

        private:
            boost::uint32_t block_size;

        private:
            just::avformat::AviFile file_;
            util::buffers::StreamBuffer<boost::uint8_t> head_buffer_;
            util::buffers::StreamBuffer<boost::uint8_t> tail_buffer_;
            AviDataContext context_;
        };

        JUST_REGISTER_MUXER("avi", AviMuxer);

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_AVI_AVI_MUXER_H_
