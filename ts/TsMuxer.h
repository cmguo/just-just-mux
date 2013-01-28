// TsMuxer.h

#ifndef _PPBOX_MUX_TS_MUXER_H_
#define _PPBOX_MUX_TS_MUXER_H_

#include "ppbox/mux/MuxerBase.h"

#include <ppbox/avformat/ts/PatPacket.h>
#include <ppbox/avformat/ts/PmtPacket.h>

namespace ppbox
{
    namespace mux
    {

        class TsMuxer
            : public MuxerBase
        {
        public:
            TsMuxer();

            virtual ~TsMuxer();

        protected:
            void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);

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

#endif // _PPBOX_MUX_TS_MUXER_H_
