// MkvMuxer.h

#ifndef _PPBOX_MUX_MKV_MKV_MUXER_H_
#define _PPBOX_MUX_MKV_MKV_MUXER_H_

#include  "ppbox/mux/MuxerBase.h"

#include <ppbox/mux/mkv/MkvTransfer.h>
#include <ppbox/avformat/mkv/MkvObjectType.h>

namespace ppbox
{
    namespace mux
    {

        class MkvMuxer
            : public MuxerBase
        {
        public:
            MkvMuxer();

            virtual ~MkvMuxer();

        private:
            virtual void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & sample);

            virtual void file_header(
                Sample & sample);

        private:
            boost::uint8_t stream_number_;
            MkvTransfer* transfer_;
            boost::asio::streambuf track_buf_;
            boost::asio::streambuf ebml_buf_;
            boost::asio::streambuf segment_buf_;
            boost::asio::streambuf track_head_buf_;
            boost::asio::streambuf cluster_buf_;
        };

    } // namespace mux
} // namespace ppbox

#endif  // _PPBOX_MUX_MKV_MKV_MUXER_H_
