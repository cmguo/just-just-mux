// FlvMuxer.h

#ifndef _PPBOX_MUX_FLV_FLV_MUXER_H_
#define _PPBOX_MUX_FLV_FLV_MUXER_H_

#include "ppbox/mux/MuxerBase.h"

#include <ppbox/avformat/flv/FlvTagType.h>
#include <ppbox/avformat/flv/FlvMetaData.h>

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

        private:
            void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);

            void file_header(
                Sample & tag);

            void stream_header(
                boost::uint32_t index, 
                Sample & tag);

        private:
            ppbox::avformat::FlvHeader flv_header_;
            ppbox::avformat::FlvMetaData meta_data_;
            boost::uint8_t header_buffer_[16];
            boost::uint8_t meta_data_buffer_[256];
            FlvTransfer * meta_data_transfer_;
            std::vector<FlvTransfer *> transfers_;
        };

        PPBOX_REGISTER_MUXER("flv", FlvMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FLV_FLV_MUXER_H_
