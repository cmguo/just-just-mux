// CodecAssemblerTransfer.cpp

#include "just/mux/Common.h"
#include "just/mux/transfer/CodecAssemblerTransfer.h"

namespace just
{
    namespace mux
    {

        CodecAssemblerTransfer::CodecAssemblerTransfer(
            boost::uint32_t codec, 
            boost::uint32_t format)
        {
            boost::system::error_code ec;
            assembler_ = just::avcodec::AssemblerFactory::create(codec, format, ec);
        }

        CodecAssemblerTransfer::~CodecAssemblerTransfer()
        {
            delete assembler_;
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
} // namespace just
