// AviMuxer.h

#ifndef _PPBOX_MUX_AVI_AVI_MUXER_H_
#define _PPBOX_MUX_AVI_AVI_MUXER_H_

#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/avi/AviDataContext.h"

#include <ppbox/avformat/avi/lib/AviFile.h>

#include <util/buffers/StreamBuffer.h>

namespace ppbox
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
            ppbox::avformat::AviFile file_;
            util::buffers::StreamBuffer<boost::uint8_t> head_buffer_;
            util::buffers::StreamBuffer<boost::uint8_t> tail_buffer_;
            AviDataContext context_;
        };

        PPBOX_REGISTER_MUXER("avi", AviMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_AVI_AVI_MUXER_H_
