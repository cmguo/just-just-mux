// TranscodeFilter.h

#ifndef _PPBOX_MUX_TRANSFER_CODEC_TRANSCODE_FILTER_H_
#define _PPBOX_MUX_TRANSFER_CODEC_TRANSCODE_FILTER_H_

#include "ppbox/mux/Filter.h"

#include <ppbox/avcodec/Transcoder.h>

namespace ppbox
{
    namespace mux
    {

        class TranscodeFilter
            : public Filter
        {
        public:
            TranscodeFilter(
                StreamInfo const & info, 
                std::vector<boost::uint32_t> const & output_codecs);

        public:
            boost::uint32_t output_codec() const;

        public:
            virtual void config(
                framework::configure::Config & conf);

        public:
            virtual bool put(
                StreamInfo & info, 
                boost::system::error_code & ec);

            virtual bool put(
                Sample & sample, 
                boost::system::error_code & ec);

            virtual bool put(
                MuxEvent const & event, 
                boost::system::error_code & ec);

        private:
            std::string encoder_param_;
            StreamInfo out_info_;
            typedef ppbox::avcodec::TranscoderFactory::transcoder_chain_t transcoder_chain_t;
            transcoder_chain_t transcoders_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_CODEC_TRANSCODE_FILTER_H_
