// CodecEncoderFilter.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/filter/CodecEncoderFilter.h"
#include "ppbox/mux/MuxError.h"

#include <framework/string/ParseStl.h>

namespace ppbox
{
    namespace mux
    {

        CodecEncoderFilter::CodecEncoderFilter(
            StreamInfo const & out_info)
            : encoder_(NULL)
            , out_info_(out_info)
        {
            encoder_ = ppbox::avcodec::Encoder::create(out_info_.sub_type);
        }

        void CodecEncoderFilter::config(
            framework::configure::Config & conf)
        {
            conf.register_module("Encoder." + StreamType::to_string(out_info_.sub_type))
                << CONFIG_PARAM_NAME_RDONLY("param", encoder_param_);
            std::map<std::string, std::string> param_map;
            boost::system::error_code ec = 
                framework::string::parse2(encoder_param_, param_map);
            encoder_->config(param_map, ec);
        }

        bool CodecEncoderFilter::put(
            StreamInfo & info, 
            boost::system::error_code & ec)
        {
            if (!encoder_->open(info, out_info_, ec))
                return false;
            info = out_info_;
            return Filter::put(info, ec);
        }

        bool CodecEncoderFilter::put(
            Sample & sample, 
            boost::system::error_code & ec)
        {
            encoder_->push(sample, ec);
            if (encoder_->pop(sample, ec)) {
                return Filter::put(sample, ec);
            } else {
                ec = error::need_more_sample;
                return false;
            }
        }

        bool CodecEncoderFilter::put(
            MuxEvent const & event, 
            boost::system::error_code & ec)
        {
            switch (event.type) {
                case MuxEvent::end:
                    {
                        Sample sample;
                        sample.stream_info = &out_info_;
                        while (encoder_->push(encoder_->eos(), ec) 
                            && encoder_->pop(sample, ec)) {
                                Filter::put(sample, ec);
                        }
                    }
                    break;
                case MuxEvent::reset:
                    encoder_->refresh(ec);
                    break;
                default:
                    break;
            }
            return Filter::put(event, ec);
        }

    } // namespace mux
} // namespace ppbox
