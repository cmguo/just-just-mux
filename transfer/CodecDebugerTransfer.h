// CodecDebugerTransfer.h

#ifndef _JUST_MUX_TRANSFER_CODEC_DEBUGER_TRANSFER_H_
#define _JUST_MUX_TRANSFER_CODEC_DEBUGER_TRANSFER_H_

#include "just/mux/Transfer.h"

#include <just/avcodec/Debuger.h>

namespace just
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
            just::avcodec::Debuger * debuger_;
            boost::uint64_t num_;
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_TRANSFER_CODEC_DEBUGER_TRANSFER_H_
