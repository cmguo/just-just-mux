// AsfMuxer.h

#ifndef _PPBOX_MUX_ASF_ASF_MUXER_H_
#define _PPBOX_MUX_ASF_ASF_MUXER_H_

#include "ppbox/mux/Muxer.h"

#include <boost/asio/streambuf.hpp>

namespace ppbox
{
    namespace mux
    {

        class AsfTransfer;

        class AsfMuxer
            : public Muxer
        {
        public:
            AsfMuxer(
                boost::asio::io_service & io_svc);

            ~AsfMuxer();

        protected:
            void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);

            void file_header(
                Sample & sample);

            void stream_header(
                boost::uint32_t index, 
                Sample & sample);

        private:
            boost::asio::streambuf file_buf_;
            boost::asio::streambuf stream_buf_;
            boost::asio::streambuf data_buf_;
            AsfTransfer * transfer_;
        };

        PPBOX_REGISTER_MUXER("asf", AsfMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_ASF_ASF_MUXER_H_
