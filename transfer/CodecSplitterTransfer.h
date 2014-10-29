// CodecSplitterTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_CODEC_SPLITTER_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_CODEC_SPLITTER_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avcodec/Splitter.h>

namespace ppbox
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
            ppbox::avcodec::Splitter * splitter_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_CODEC_SPLITTER_TRANSFER_H_
