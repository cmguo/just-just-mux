// RawMuxer.h

#ifndef _PPBOX_MUX_RAW_RAW_MUXER_H_
#define _PPBOX_MUX_RAW_RAW_MUXER_H_

#include "ppbox/mux/Muxer.h"

namespace ppbox
{
    namespace mux
    {

        class RawFormat;

        class RawMuxer
            : public Muxer
        {
        public:
            RawMuxer(
                boost::asio::io_service & io_svc);

            virtual ~RawMuxer();

        private:
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

        private:
            RawFormat * format_;
        };

        PPBOX_REGISTER_MUXER("raw", RawMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RAW_RAW_MUXER_H_
