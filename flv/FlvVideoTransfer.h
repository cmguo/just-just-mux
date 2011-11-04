// FlvVideoTransfer.h

#ifndef   _PPBOX_MUX_FLV_VIDEO_TRANSFER_H_
#define   _PPBOX_MUX_FLV_VIDEO_TRANSFER_H_

#include "ppbox/mux/flv/FlvMetadata.h"
#include "ppbox/mux/transfer/Transfer.h"

namespace ppbox
{
    namespace mux
    {

        class FlvVideoTransfer
            : public Transfer
        {
        public:
            FlvVideoTransfer()
                : previous_tag_size_(0)
            {
            }

            ~FlvVideoTransfer()
            {
            }

            virtual void transfer(ppbox::demux::Sample & sample);

        private:
            FlvTag    flvtag_;
            boost::uint32_t previous_tag_size_;
            std::vector<boost::uint8_t> sample_head_buffer_;
        };
    }
}
#endif // _PPBOX_MUX_FLV_TRANSFER_H_
