// FlvMuxer.h

#ifndef _JUST_MUX_FLV_FLV_MUXER_H_
#define _JUST_MUX_FLV_FLV_MUXER_H_

#include "just/mux/Muxer.h"

#include <just/avformat/flv/FlvTagType.h>
#include <just/avformat/flv/FlvMetaData.h>

#include <util/buffers/StreamBuffer.h>

namespace just
{
    namespace mux
    {

        class FlvTransfer;

        class FlvMuxer
            : public Muxer
        {
        public:
            FlvMuxer(
                boost::asio::io_service & io_svc);

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
            just::avformat::FlvHeader flv_header_;
            just::avformat::FlvMetaData meta_data_;
            boost::uint8_t header_buffer_[16];
            //boost::uint8_t meta_data_buffer_[256];
            util::buffers::StreamBuffer<boost::uint8_t> meta_data_buffer_;
            FlvTransfer * meta_data_transfer_;
            std::vector<FlvTransfer *> transfers_;
        };

        JUST_REGISTER_MUXER("flv", FlvMuxer);

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_FLV_FLV_MUXER_H_
