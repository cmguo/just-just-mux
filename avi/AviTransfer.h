// AviTransfer.h

#ifndef _JUST_MUX_AVI_AVI_TRANSFER_H_
#define _JUST_MUX_AVI_AVI_TRANSFER_H_

#include "just/mux/MuxBase.h"
#include "just/mux/Transfer.h"

#include <just/avformat/avi/lib/AviStream.h>

namespace just
{
    namespace mux
    {

        class AviDataContext;

        class AviTransfer
            : public Transfer
        {
        public:
            AviTransfer(
                just::avformat::AviStream * stream, 
                AviDataContext * ctx);

            virtual ~AviTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

            virtual void on_event(
                MuxEvent const & event);

        private:
            just::avformat::AviStream * stream_;
            AviDataContext * ctx_;
            boost::uint32_t buffer_[2];
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_AVI_AVI_TRANSFER_H_
