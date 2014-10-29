// AviTransfer.h

#ifndef _PPBOX_MUX_AVI_AVI_TRANSFER_H_
#define _PPBOX_MUX_AVI_AVI_TRANSFER_H_

#include "ppbox/mux/MuxBase.h"
#include "ppbox/mux/Transfer.h"

#include <ppbox/avformat/avi/lib/AviStream.h>

namespace ppbox
{
    namespace mux
    {

        class AviDataContext;

        class AviTransfer
            : public Transfer
        {
        public:
            AviTransfer(
                ppbox::avformat::AviStream * stream, 
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
            ppbox::avformat::AviStream * stream_;
            AviDataContext * ctx_;
            boost::uint32_t buffer_[2];
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_AVI_AVI_TRANSFER_H_
