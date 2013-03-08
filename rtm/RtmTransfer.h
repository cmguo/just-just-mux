// RtmTransfer.h

#ifndef _PPBOX_MUX_RTM_RTM_TRANSFER_H_
#define _PPBOX_MUX_RTM_RTM_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <util/protocol/rtmp/RtmpChunkHeader.h>

namespace ppbox
{
    namespace mux
    {

        class RtmTransfer
            : public Transfer
        {
        public:
            RtmTransfer();

            ~RtmTransfer();

        public:
            virtual void transfer(
                Sample & sample);

            virtual void on_seek(
                boost::uint64_t time);

        private:
            boost::uint8_t header_buffer_[20];
            util::protocol::RtmpChunkHeader header_;
            boost::uint32_t chunk_size_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTM_RTM_TRANSFER_H_
