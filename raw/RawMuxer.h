// RawMuxer.h

#ifndef _PPBOX_MUX_RAW_RAW_MUXER_H_
#define _PPBOX_MUX_RAW_RAW_MUXER_H_

#include "ppbox/mux/MuxerBase.h"

namespace ppbox
{
    namespace mux
    {

        class RawFormat;

        class RawMuxer
            : public MuxerBase
        {
        public:
            RawMuxer();

            virtual ~RawMuxer();

        private:
            virtual void do_open(
                MediaInfo & info);

            virtual void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);

            virtual void file_header(
                Sample & sample);

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & sample);

        private:
            RawFormat * format_;
        };

        PPBOX_REGISTER_MUXER("raw", RawMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RAW_RAW_MUXER_H_
