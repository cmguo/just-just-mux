// CodecSplitterTransfer.h

#ifndef _JUST_MUX_TRANSFER_CODEC_SPLITTER_TRANSFER_H_
#define _JUST_MUX_TRANSFER_CODEC_SPLITTER_TRANSFER_H_

#include "just/mux/Transfer.h"

#include <just/avcodec/Splitter.h>

namespace just
{
    namespace mux
    {

        class CodecSplitterTransfer
            : public Transfer
        {
        public:
            CodecSplitterTransfer(
                boost::uint32_t codec, 
                boost::uint32_t format);

            virtual ~CodecSplitterTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            just::avcodec::Splitter * splitter_;
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_TRANSFER_CODEC_SPLITTER_TRANSFER_H_
