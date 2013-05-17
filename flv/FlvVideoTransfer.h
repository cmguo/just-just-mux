// FlvVideoTransfer.h

#ifndef _PPBOX_MUX_FLV_FLV_VIDEO_TRANSFER_H_
#define _PPBOX_MUX_FLV_FLV_VIDEO_TRANSFER_H_

#include "ppbox/mux/flv/FlvTransfer.h"

namespace ppbox
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
            ppbox::avformat::FlvVideoTagHeader header_;
            boost::uint8_t header_buffer_[16];
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FLV_FLV_VIDEO_TRANSFER_H_
