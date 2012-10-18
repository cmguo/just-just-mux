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
            FlvVideoTransfer(
                boost::uint8_t type);

        public:
            virtual void transfer(
                Sample & sample);

            virtual void transfer(
                StreamInfo & info);

        private:
            ppbox::avformat::FlvVideoTagHeader videotagheader_;
            char video_tag_header_[16];
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FLV_FLV_VIDEO_TRANSFER_H_
