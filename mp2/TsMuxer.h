// TsMuxer.h

#ifndef _PPBOX_MUX_MP2_MUXER_H_
#define _PPBOX_MUX_MP2_MUXER_H_

#include "ppbox/mux/Muxer.h"

#include <ppbox/avformat/mp2/PatPacket.h>
#include <ppbox/avformat/mp2/PmtPacket.h>

namespace ppbox
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
            ppbox::avformat::PatPacket pat_;
            ppbox::avformat::PmtPacket pmt_;
            boost::uint8_t header_buffer_[512];
        };

        PPBOX_REGISTER_MUXER("ts", TsMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MP2_MUXER_H_
