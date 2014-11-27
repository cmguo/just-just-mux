// FlvTransfer.h

#ifndef _JUST_MUX_FLV_FLV_TRANSFER_H_
#define _JUST_MUX_FLV_FLV_TRANSFER_H_

#include "just/mux/MuxBase.h"
#include "just/mux/Transfer.h"

#include <just/avformat/flv/FlvFormat.h>
#include <just/avformat/flv/FlvTagType.h>

#include <util/archive/ArchiveBuffer.h>

#include <framework/system/BytesOrder.h>

namespace just
{
    namespace mux
    {

        class FlvTransfer
            : public Transfer
        {
        public:
            FlvTransfer(
                boost::uint8_t type);

            virtual ~FlvTransfer();

        public:
            virtual void transfer(
                Sample & sample);

        public:
            virtual void stream_header(
                StreamInfo const & info, 
                Sample & sample);

        private:
            just::avformat::FlvTagHeader header_;
            boost::uint8_t header_buffer_[16];
            boost::uint32_t previous_tag_size_;
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_FLV_FLV_TRANSFER_H_
