// ParseH264Transfer.h

#ifndef _PPBOX_MUX_TRANSFER_PARSE_H264_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_PARSE_H264_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/tinyvlc.h"

namespace ppbox
{
    namespace mux
    {
        class ParseH264Transfer
            : public Transfer
        {
        public:
            ParseH264Transfer()
            {
            }

            ~ParseH264Transfer()
            {
            }

            void transfer(ppbox::demux::Sample & sample);

        private:
            NaluParser nalu_parser_;
        };
    }
}

#endif // _PPBOX_MUX_TRANSFER_PARSE_H264_TRANSFER_H_
