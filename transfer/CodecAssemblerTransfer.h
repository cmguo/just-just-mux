// CodecAssemblerTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_CODEC_ASSEMBLER_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_CODEC_ASSEMBLER_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avcodec/Assembler.h>

namespace ppbox
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
            ppbox::avcodec::Assembler * assembler_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_CODEC_ASSEMBLER_TRANSFER_H_
