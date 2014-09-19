// TranscodeFilter.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/filter/TranscodeFilter.h"

#include <framework/string/ParseStl.h>

namespace ppbox
{
    namespace mux
    {

        TranscodeFilter::TranscodeFilter(
            StreamInfo const & info, 
            std::vector<boost::uint32_t> const & output_codecs)
            : out_info_(info)
        {
            std::vector<boost::uint32_t>::const_iterator iter = 
                std::find(output_codecs.begin(), output_codecs.end(), info.sub_type);
            if (iter != output_codecs.end()) {
                out_info_.sub_type = info.sub_type;
                return;
            }

            out_info_.sub_type = ppbox::avbase::StreamSubType::NONE;
            out_info_.format_type = ppbox::avbase::StreamFormatType::none;

            boost::system::error_code ec;
            if (ppbox::avcodec::TranscoderFactory::create_transcodes(
                info.type, 
                info.sub_type, 
                output_codecs, 
                transcoders_, 
                4, 
                ec)) {
                    out_info_.sub_type = transcoders_.back().codec;
            }
        }

        boost::uint32_t TranscodeFilter::output_codec() const
        {
            return out_info_.sub_type;
        }

        void TranscodeFilter::config(
            framework::configure::Config & conf)
        {
            for (size_t i = 0; i < transcoders_.size(); ++i) {
                conf.register_module("Transcoder." + transcoders_[i].key)
                    << CONFIG_PARAM_NAME_RDONLY("param", encoder_param_);
                std::map<std::string, std::string> param_map;
                boost::system::error_code ec = 
                    framework::string::parse2(encoder_param_, param_map);
                transcoders_[i].transcoder->config(param_map, ec);
            }
        }

        bool TranscodeFilter::put(
            StreamInfo & info, 
            boost::system::error_code & ec)
        {
            StreamInfo info2 = info;
            for (size_t i = 0; i < transcoders_.size(); ++i) {
                info2.sub_type = transcoders_[i].codec;
                info2.format_type = 0;
                info2.format_data.clear();
                if (!transcoders_[i].transcoder->open(info, info2, ec))
                    return false;
                info = info2;
            }
            return Filter::put(info, ec);
        }

        bool TranscodeFilter::put(
            Sample & sample, 
            boost::system::error_code & ec)
        {
            for (size_t i = 0; i < transcoders_.size(); ++i) {
                if (transcoders_[i].transcoder->push(sample, ec)) { 
                    if (transcoders_[i].transcoder->pop(sample, ec)) {
                        continue;
                    } else {
                        return true;
                    }
                } else {
                    return false;
                }
            }
            return Filter::put(sample, ec);
        }

        bool TranscodeFilter::put(
            MuxEvent const & event, 
            boost::system::error_code & ec)
        {
            switch (event.type) {
                case MuxEvent::end:
                    for (size_t i = 0; i < transcoders_.size(); ++i) {
                        if (transcoders_[i].transcoder->push(ppbox::avcodec::Transcoder::eos(), ec)) { 
                            Sample sample;
                            sample.stream_info = &out_info_;
                            if (transcoders_[i].transcoder->pop(sample, ec)) {
                                for (++i; i < transcoders_.size(); ++i) {
                                    if (transcoders_[i].transcoder->push(sample, ec)) { 
                                        if (transcoders_[i].transcoder->pop(sample, ec)) {
                                            continue;
                                        } else {
                                            return true;
                                        }
                                    } else {
                                        return false;
                                    }
                                }
                                return Filter::put(sample, ec);
                            } else {
                                return false;
                            }
                        } else {
                            continue;
                        }
                    }
                    break;
                case MuxEvent::after_reset:
                    for (size_t i = 0; i < transcoders_.size(); ++i) {
                        if (!transcoders_[i].transcoder->refresh(ec))
                            return false;
                    }
                    break;
                default:
                    break;
            }
            return Filter::put(event, ec);
        }

    } // namespace mux
} // namespace ppbox
