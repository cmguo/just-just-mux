// CodecDebugerTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/CodecDebugerTransfer.h"

namespace ppbox
{
    namespace mux
    {

        CodecDebugerTransfer::CodecDebugerTransfer(
            boost::uint32_t codec)
            : num_(0)
        {
            boost::system::error_code ec;
            debuger_ = ppbox::avcodec::Debuger::create(codec, ec);
        }

        void CodecDebugerTransfer::transfer(
            StreamInfo & info)
        {
            boost::system::error_code ec;
            debuger_->reset(info, ec);
            num_ = 0;
        }

        void CodecDebugerTransfer::transfer(
            Sample & sample)
        {
            boost::system::error_code ec;
            std::cout << "track: " << sample.itrack
                << " # " << num_++
                << "\t time: " << sample.time 
                << "\t size: " << sample.size 
                << "\t dts: " << sample.dts 
                << "\t cts: " << sample.dts + sample.cts_delta 
                << std::endl;
            debuger_->debug(sample, ec);
        }

    } // namespace mux
} // namespace ppbox
