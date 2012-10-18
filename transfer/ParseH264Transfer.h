// ParseH264Transfer.h

#ifndef _PPBOX_MUX_TRANSFER_PARSE_H264_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_PARSE_H264_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

namespace ppbox
{
    namespace mux
    {

        class ParseH264Transfer
            : public Transfer
        {
        public:
            virtual void transfer(
                StreamInfo & media);

            virtual void transfer(
                Sample & sample);
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_PARSE_H264_TRANSFER_H_
