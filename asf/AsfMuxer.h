// AsfMuxer.h

#ifndef _PPBOX_MUX_ASF_MUXER_H_
#define _PPBOX_MUX_ASF_MUXER_H_

#include  "ppbox/mux/MuxerBase.h"

#include  <boost/asio/streambuf.hpp>
#include  <ppbox/avformat/asf/AsfGuid.h>
#include  <ppbox/avformat/asf/AsfObjectType.h>

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
                MediaInfoEx & mediainfoex);

            void stream_header(
                boost::uint32_t index, 
                ppbox::demux::Sample & tag);

            void file_header(
                ppbox::demux::Sample & tag);

            boost::system::error_code seek(
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

#endif // _PPBOX_MUX_ASF_MUXER_H_
