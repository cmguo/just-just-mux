// FlvVideoTransfer.h

#ifndef _JUST_MUX_FLV_FLV_VIDEO_TRANSFER_H_
#define _JUST_MUX_FLV_FLV_VIDEO_TRANSFER_H_

#include "just/mux/flv/FlvTransfer.h"

namespace just
{
    namespace mux
    {

        class FlvVideoTransfer
            : public FlvTransfer
        {
        public:
            FlvVideoTransfer();

        public:
            virtual void transfer(
                Sample & sample);

            virtual void transfer(
                StreamInfo & info);

        public:
            virtual void stream_header(
                StreamInfo const & info, 
                Sample & sample);

        private:
            just::avformat::FlvVideoTagHeader header_;
            boost::uint8_t header_buffer_[16];
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_FLV_FLV_VIDEO_TRANSFER_H_
