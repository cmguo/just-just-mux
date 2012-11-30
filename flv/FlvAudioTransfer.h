// FlvAudioTransfer.h

#ifndef _PPBOX_MUX_FLV_FLV_AUDIO_TRANSFER_H_
#define _PPBOX_MUX_FLV_FLV_AUDIO_TRANSFER_H_

#include "ppbox/mux/flv/FlvTransfer.h"

namespace ppbox
{
    namespace mux
    {

        class FlvAudioTransfer
            : public FlvTransfer
        {
        public:
            FlvAudioTransfer(
                boost::uint8_t type);

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        public:
            virtual void stream_header(
                StreamInfo const & info, 
                Sample & sample);

        private:
            ppbox::avformat::FlvAudioTagHeader header_;
            std::vector<boost::uint8_t> config_data_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FLV_FLV_AUDIO_TRANSFER_H_
