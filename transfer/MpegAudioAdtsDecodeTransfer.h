// MpegAudioAdtsDecodeTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_MPEG_AUDIO_ADTS_DECODE_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_MPEG_AUDIO_ADTS_DECODE_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

namespace ppbox
{
    namespace mux
    {

        class MpegAudioAdtsDecodeTransfer
            : public Transfer
        {
        public:
            MpegAudioAdtsDecodeTransfer();

            ~MpegAudioAdtsDecodeTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_MPEG_AUDIO_ADTS_DECODE_TRANSFER_H_
