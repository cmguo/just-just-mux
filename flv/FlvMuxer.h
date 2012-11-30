// FlvMuxer.h

#ifndef _PPBOX_MUX_FLV_FLV_MUXER_H_
#define _PPBOX_MUX_FLV_FLV_MUXER_H_

#include "ppbox/mux/MuxerBase.h"

#include <ppbox/avformat/flv/FlvTagType.h>

namespace ppbox
{
    namespace mux
    {

        class FlvTransfer;

        class FlvMuxer
            : public MuxerBase
        {
        public:
            FlvMuxer();

            ~FlvMuxer();

        public:
            void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);

            void file_header(
                Sample & tag);

            void stream_header(
                boost::uint32_t index, 
                Sample & tag);

        private:
            boost::uint8_t header_buffer_[16];
            std::vector<FlvTransfer *> transfers_;
        };

        PPBOX_REGISTER_MUXER("flv", FlvMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FLV_FLV_MUXER_H_
