// Mp4Transfer.h

#ifndef _JUST_MUX_MP4_MP4_TRANSFER_H_
#define _JUST_MUX_MP4_MP4_TRANSFER_H_

#include "just/mux/MuxBase.h"
#include "just/mux/Transfer.h"

#include <just/avformat/mp4/lib/Mp4Track.h>

namespace just
{
    namespace mux
    {

        class Mp4DataContext;

        class Mp4Transfer
            : public Transfer
        {
        public:
            Mp4Transfer(
                just::avformat::Mp4Track * track, 
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
            just::avformat::Mp4Track * track_;
            just::avformat::Mp4SampleTable * table_;
            Mp4DataContext * ctx_;
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_MP4_MP4_TRANSFER_H_
