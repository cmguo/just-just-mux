// MkvMuxer.h

#ifndef _PPBOX_MUX_MKV_MKV_MUXER_H_
#define _PPBOX_MUX_MKV_MKV_MUXER_H_

#include  "ppbox/mux/Muxer.h"

#include <boost/asio/streambuf.hpp>

namespace ppbox
{
    namespace mux
    {

        class MkvTransfer;

        class MkvMuxer
            : public Muxer
        {
        public:
            MkvMuxer(
                boost::asio::io_service & io_svc);

            virtual ~MkvMuxer();

        private:
            virtual void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);

            virtual void file_header(
                Sample & sample);

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & sample);

        private:
            boost::uint8_t stream_number_;
            MkvTransfer * transfer_;
            boost::asio::streambuf header_buf_;
            boost::asio::streambuf track_buf_;
        };

        PPBOX_REGISTER_MUXER("mkv", MkvMuxer);

    } // namespace mux
} // namespace ppbox

#endif  // _PPBOX_MUX_MKV_MKV_MUXER_H_
