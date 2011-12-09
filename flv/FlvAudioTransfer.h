// FlvAudioTransfer.h

#ifndef   _PPBOX_MUX_FLV_AUDIO_TRANSFER_H_
#define   _PPBOX_MUX_FLV_AUDIO_TRANSFER_H_

#include "ppbox/mux/flv/FlvTransfer.h"

namespace ppbox
{
    namespace mux
    {

        class FlvAudioTransfer
            : public FlvTransfer
        {
        public:
            FlvAudioTransfer(boost::uint8_t type)
                : FlvTransfer(type)
            {
            }

            ~FlvAudioTransfer()
            {
            }

            virtual void transfer(ppbox::demux::Sample & sample);

            virtual void transfer(MediaInfoEx & mediainfo);

        private:
            ppbox::demux::FlvAudioTagHeader audiotagheader_;

        };
    }
}
#endif // _PPBOX_MUX_FLV_AUDIO_TRANSFER_H_
