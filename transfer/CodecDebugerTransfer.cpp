// CodecDebugerTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/CodecDebugerTransfer.h"

namespace ppbox
{
    namespace mux
    {

        CodecDebugerTransfer::CodecDebugerTransfer(
            boost::uint32_t codec)
        {
            ppbox::avcodec::Debuger::init();
            debuger_ = ppbox::avcodec::Debuger::create(codec);
        }

        void CodecDebugerTransfer::transfer(
            StreamInfo & info)
        {
            boost::system::error_code ec;
            debuger_->reset(info, ec);
        }

        void CodecDebugerTransfer::transfer(
            Sample & sample)
        {
            boost::system::error_code ec;
            debuger_->debug(sample, ec);
        }

    } // namespace mux
} // namespace ppbox
