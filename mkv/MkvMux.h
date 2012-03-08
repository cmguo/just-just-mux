//MkvMux.h
#ifndef _PPBOX_MUX_MKV_MKV_MUX_H_
#define _PPBOX_MUX_MKV_MKV_MUX_H_

#include  "ppbox/mux/Muxer.h"

#include <ppbox/mux/mkv/MkvTransfer.h>
#include <ppbox/avformat/mkv/MkvObjectType.h>

namespace ppbox
{
    namespace mux
    {

        class MkvMux
            : public Muxer
        {
        public:
            MkvMux();

            ~MkvMux();

        public:
            void add_stream(
                MediaInfoEx & mediainfoex);

            void stream_header(
                boost::uint32_t index, 
                ppbox::demux::Sample & tag);

            void file_header(
                ppbox::demux::Sample & tag);

            boost::system::error_code seek(
                boost::uint32_t & time,
                boost::system::error_code & ec);

        private:
            boost::uint8_t stream_number_;
            MkvTransfer* transfer_;
            boost::asio::streambuf track_buf_;
            boost::asio::streambuf ebml_buf_;
            boost::asio::streambuf segment_buf_;
            boost::asio::streambuf track_head_buf_;
            boost::asio::streambuf cluster_buf_;
        };
    }
}






#endif


