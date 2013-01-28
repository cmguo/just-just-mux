// MmsTransfer.h

#ifndef _PPBOX_MUX_MMS_MMS_TRANSFER_H_
#define _PPBOX_MUX_MMS_MMS_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <util/protocol/mmsp/MmspData.h>

namespace ppbox
{
    namespace mux
    {

        class MmsTransfer
            : public Transfer
        {
        public:
            MmsTransfer();

            ~MmsTransfer();

        public:
            virtual void transfer(
                Sample & sample);

        public:
            void file_header(
                Sample & sample);

        private:
            std::vector<boost::uint8_t> header_buffer_;
            util::protocol::MmspDataHeader header_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MMS_MMS_TRANSFER_H_
