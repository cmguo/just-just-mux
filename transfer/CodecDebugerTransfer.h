// CodecDebugerTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_CODEC_DEBUGER_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_CODEC_DEBUGER_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avcodec/Debuger.h>

namespace ppbox
{
    namespace mux
    {

        class CodecDebugerTransfer
            : public Transfer
        {
        public:
            CodecDebugerTransfer(
                boost::uint32_t codec);

            virtual ~CodecDebugerTransfer();

        public:
            virtual void transfer(
                StreamInfo & media);

            virtual void transfer(
                Sample & sample);

        private:
            ppbox::avcodec::Debuger * debuger_;
            boost::uint64_t num_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_CODEC_DEBUGER_TRANSFER_H_
