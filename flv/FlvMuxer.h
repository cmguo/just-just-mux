// FlvMuxer.h

#ifndef _PPBOX_MUX_FLV_FLV_MUXER_H_
#define _PPBOX_MUX_FLV_FLV_MUXER_H_

#include "ppbox/mux/MuxerBase.h"

#include <ppbox/avformat/flv/FlvTagType.h>
#include <ppbox/avformat/flv/FlvMetaData.h>

#include <util/buffers/StreamBuffer.h>

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

        protected:
            virtual void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);

            virtual void file_header(
                Sample & sample);

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & sample);

        private:
            ppbox::avformat::FlvHeader flv_header_;
            ppbox::avformat::FlvMetaData meta_data_;
            boost::uint8_t header_buffer_[16];
            //boost::uint8_t meta_data_buffer_[256];
            util::buffers::StreamBuffer<boost::uint8_t> meta_data_buffer_;
            FlvTransfer * meta_data_transfer_;
            std::vector<FlvTransfer *> transfers_;
        };

        PPBOX_REGISTER_MUXER("flv", FlvMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FLV_FLV_MUXER_H_
