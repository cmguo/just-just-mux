// FlvAudioTransfer.h

#ifndef _JUST_MUX_FLV_FLV_AUDIO_TRANSFER_H_
#define _JUST_MUX_FLV_FLV_AUDIO_TRANSFER_H_

#include "just/mux/flv/FlvTransfer.h"

namespace just
{
    namespace mux
    {

        class FlvAudioTransfer
            : public FlvTransfer
        {
        public:
            FlvAudioTransfer();

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
            just::avformat::FlvAudioTagHeader header_;
            boost::uint8_t header_buffer_[16];
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_FLV_FLV_AUDIO_TRANSFER_H_
