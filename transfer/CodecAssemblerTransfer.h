// CodecAssemblerTransfer.h

#ifndef _JUST_MUX_TRANSFER_CODEC_ASSEMBLER_TRANSFER_H_
#define _JUST_MUX_TRANSFER_CODEC_ASSEMBLER_TRANSFER_H_

#include "just/mux/Transfer.h"

#include <just/avcodec/Assembler.h>

namespace just
{
    namespace mux
    {

        class CodecAssemblerTransfer
            : public Transfer
        {
        public:
            CodecAssemblerTransfer(
                boost::uint32_t codec, 
                boost::uint32_t format);

            virtual ~CodecAssemblerTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            just::avcodec::Assembler * assembler_;
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_TRANSFER_CODEC_ASSEMBLER_TRANSFER_H_
