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
            debuger_ = ppbox::avcodec::DebugerFactory::create(codec, ec);
            if (debuger_ == NULL)
                debuger_ = new ppbox::avcodec::Debuger;
        }

        CodecDebugerTransfer::~CodecDebugerTransfer()
        {
            delete debuger_;
        }

        void CodecDebugerTransfer::transfer(
            StreamInfo & info)
        {
            std::cout << "track: " << info.index
                << "\t type: " << ppbox::avbase::FourCC::to_string(info.type)
                << "\t sub_type: " << ppbox::avbase::FourCC::to_string(info.sub_type)
                << "\t format_data: " << info.format_data.size() << " bytes" 
                << std::endl;
            if (info.type == StreamType::VIDE) {
                std::cout << "  width: " << info.video_format.width
                    << " height: " << info.video_format.height
                    << " frame_rate: " << info.video_format.frame_rate_num << "/" << info.video_format.frame_rate_den
                    << std::endl;
            } else {
                std::cout << "  channel_count: " << info.audio_format.channel_count
                    << " sample_size: " << info.audio_format.sample_size
                    << " sample_rate: " << info.audio_format.sample_rate
                    << " block_align: " << info.audio_format.block_align
                    << " sample_per_frame: " << info.audio_format.sample_per_frame
                    << std::endl;
            }
            boost::system::error_code ec;
            if (debuger_)
                 debuger_->reset(info, ec);
            num_ = 0;
        }

        void CodecDebugerTransfer::transfer(
            Sample & sample)
        {
            std::cout << "track: " << sample.itrack
                << " # " << num_++
                << "\t time: " << sample.time 
                << "\t size: " << sample.size 
                << "\t dts: " << sample.dts 
                << "\t cts: " << sample.dts + sample.cts_delta 
                << std::endl;
            boost::system::error_code ec;
            if (debuger_)
                debuger_->debug(sample, ec);
        }

    } // namespace mux
} // namespace ppbox
