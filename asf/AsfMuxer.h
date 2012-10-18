// AsfMuxer.h

#ifndef _PPBOX_MUX_ASF_ASF_MUXER_H_
#define _PPBOX_MUX_ASF_ASF_MUXER_H_

#include "ppbox/mux/MuxerBase.h"

#include <boost/asio/streambuf.hpp>

namespace ppbox
{
    namespace mux
    {

        class AsfTransfer;

        class AsfMuxer
            : public MuxerBase
        {
        public:
            AsfMuxer();

            ~AsfMuxer();

        public:
            void add_stream(
                StreamInfo & infoex);

            void stream_header(
                boost::uint32_t index, 
                Sample & tag);

            void file_header(
                Sample & tag);

            boost::system::error_code time_seek(
                boost::uint64_t & time,
                boost::system::error_code & ec);

        private:
            boost::asio::streambuf extension_buf_;
            boost::asio::streambuf head_buf_;
            boost::asio::streambuf file_buf_;
            boost::asio::streambuf stream_buf_;
            boost::asio::streambuf data_buf_;
            boost::uint8_t stream_number_;
            AsfTransfer * transfer_;
        };

        PPBOX_REGISTER_MUXER(asf, AsfMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_ASF_ASF_MUXER_H_
