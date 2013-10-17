// CodecAssemblerTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/CodecAssemblerTransfer.h"

namespace ppbox
{
    namespace mux
    {

        CodecAssemblerTransfer::CodecAssemblerTransfer(
            boost::uint32_t codec, 
            boost::uint32_t format)
        {
            boost::system::error_code ec;
            assembler_ = ppbox::avcodec::AssemblerFactory::create(codec, format, ec);
        }

        void CodecAssemblerTransfer::transfer(
            StreamInfo & info)
        {
            boost::system::error_code ec;
            assembler_->reset(info, ec);
        }

        void CodecAssemblerTransfer::transfer(
            Sample & sample)
        {
            boost::system::error_code ec;
            assembler_->assemble(sample, ec);
        }

    } // namespace mux
} // namespace ppbox
