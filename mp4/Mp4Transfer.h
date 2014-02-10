// Mp4Transfer.h

#ifndef _PPBOX_MUX_MP4_MP4_TRANSFER_H_
#define _PPBOX_MUX_MP4_MP4_TRANSFER_H_

#include "ppbox/mux/MuxBase.h"
#include "ppbox/mux/Transfer.h"

#include <ppbox/avformat/mp4/lib/Mp4Track.h>

namespace ppbox
{
    namespace mux
    {

        class Mp4DataContext;

        class Mp4Transfer
            : public Transfer
        {
        public:
            Mp4Transfer(
                ppbox::avformat::Mp4Track * track, 
                Mp4DataContext * ctx);

            virtual ~Mp4Transfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

            virtual void on_event(
                MuxEvent const & event);

        private:
            ppbox::avformat::Mp4Track * track_;
            ppbox::avformat::Mp4SampleTable * table_;
            Mp4DataContext * ctx_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MP4_MP4_TRANSFER_H_
