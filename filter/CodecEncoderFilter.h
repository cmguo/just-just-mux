// CodecEncoderFilter.h

#ifndef _PPBOX_MUX_TRANSFER_CODEC_ENCODER_FILTER_H_
#define _PPBOX_MUX_TRANSFER_CODEC_ENCODER_FILTER_H_

#include "ppbox/mux/Filter.h"

#include <ppbox/avcodec/Encoder.h>

namespace ppbox
{
    namespace mux
    {

        class CodecEncoderFilter
            : public Filter
        {
        public:
            CodecEncoderFilter(
                StreamInfo const & out_info);

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
            ppbox::avcodec::Encoder * encoder_;
            std::string encoder_param_;
            StreamInfo out_info_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_CODEC_ENCODER_FILTER_H_
