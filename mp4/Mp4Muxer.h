// Mp4Muxer.h

#ifndef _JUST_MUX_MP4_MP4_MUXER_H_
#define _JUST_MUX_MP4_MP4_MUXER_H_

#include "just/mux/Muxer.h"
#include "just/mux/mp4/Mp4DataContext.h"

#include <just/avformat/mp4/lib/Mp4File.h>

#include <util/buffers/StreamBuffer.h>

namespace just
{
    namespace mux
    {

        class Mp4Transfer;

        class Mp4Muxer
            : public Muxer
        {
        public:
            Mp4Muxer(
                boost::asio::io_service & io_svc);

            ~Mp4Muxer();

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
            just::avformat::Mp4File file_;
            util::buffers::StreamBuffer<boost::uint8_t> head_buffer_;
            util::buffers::StreamBuffer<boost::uint8_t> tail_buffer_;
            Mp4DataContext context_;
        };

        JUST_REGISTER_MUXER("mp4", Mp4Muxer);

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_MP4_MP4_MUXER_H_
