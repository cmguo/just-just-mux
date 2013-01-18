// FlvTransfer.h

#ifndef _PPBOX_MUX_FLV_FLV_TRANSFER_H_
#define _PPBOX_MUX_FLV_FLV_TRANSFER_H_

#include "ppbox/mux/MuxBase.h"
#include "ppbox/mux/Transfer.h"

#include <ppbox/avformat/flv/FlvFormat.h>
#include <ppbox/avformat/flv/FlvTagType.h>

#include <util/archive/ArchiveBuffer.h>

#include <framework/system/BytesOrder.h>

namespace ppbox
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
            ppbox::avformat::FlvTagHeader header_;
            boost::uint8_t header_buffer_[16];
            boost::uint32_t previous_tag_size_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FLV_FLV_TRANSFER_H_
