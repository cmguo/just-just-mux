// CodecSplitterTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/CodecSplitterTransfer.h"

namespace ppbox
{
    namespace mux
    {

        CodecSplitterTransfer::CodecSplitterTransfer(
            boost::uint32_t codec, 
            boost::uint32_t format)
        {
            boost::system::error_code ec;
            splitter_ = ppbox::avcodec::SplitterFactory::create(codec, format, ec);
        }

        void CodecSplitterTransfer::~CodecSplitterTransfer()
        {
            delete splitter_;
        }

        void CodecSplitterTransfer::transfer(
            StreamInfo & info)
        {
            boost::system::error_code ec;
            splitter_->reset(info, ec);
        }

        void CodecSplitterTransfer::transfer(
            Sample & sample)
        {
            boost::system::error_code ec;
            splitter_->split(sample, ec);
        }

    } // namespace mux
} // namespace ppbox
