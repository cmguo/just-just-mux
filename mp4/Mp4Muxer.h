// Mp4Muxer.h

#ifndef _PPBOX_MUX_MP4_MP4_MUXER_H_
#define _PPBOX_MUX_MP4_MP4_MUXER_H_

#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/mp4/Mp4DataContext.h"

#include <ppbox/avformat/mp4/lib/Mp4File.h>

#include <util/buffers/StreamBuffer.h>

namespace ppbox
{
    namespace mux
    {

        class Mp4Transfer;

        class Mp4Muxer
            : public Muxer
        {
        public:
            Mp4Muxer();

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
            ppbox::avformat::Mp4File file_;
            util::buffers::StreamBuffer<boost::uint8_t> head_buffer_;
            util::buffers::StreamBuffer<boost::uint8_t> tail_buffer_;
            Mp4DataContext context_;
        };

        PPBOX_REGISTER_MUXER("mp4", Mp4Muxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MP4_MP4_MUXER_H_
