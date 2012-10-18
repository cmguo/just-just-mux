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

        private:
            ppbox::avformat::FlvAudioTagHeader audiotagheader_;

        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FLV_FLV_AUDIO_TRANSFER_H_
