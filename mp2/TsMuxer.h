// TsMuxer.h

#ifndef _JUST_MUX_MP2_MUXER_H_
#define _JUST_MUX_MP2_MUXER_H_

#include "just/mux/Muxer.h"

#include <just/avformat/mp2/PatPacket.h>
#include <just/avformat/mp2/PmtPacket.h>

namespace just
{
    namespace mux
    {

        class TsMuxer
            : public Muxer
        {
        public:
            TsMuxer(
                boost::asio::io_service & io_svc);

            virtual ~TsMuxer();

        protected:
            void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);

            void file_header(
                Sample & tag);

            void stream_header(
                boost::uint32_t index, 
                Sample & tag);

        private:
            just::avformat::PatPacket pat_;
            just::avformat::PmtPacket pmt_;
            boost::uint8_t header_buffer_[512];
        };

        JUST_REGISTER_MUXER("ts", TsMuxer);

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_MP2_MUXER_H_
