// RawMuxer.h

#ifndef _PPBOX_MUX_RAW_RAW_MUXER_H_
#define _PPBOX_MUX_RAW_RAW_MUXER_H_

#include "ppbox/mux/MuxerBase.h"

namespace ppbox
{
    namespace mux
    {

        class RawMuxer
            : public MuxerBase
        {
        public:
            RawMuxer();

            virtual ~RawMuxer();

        public:
            virtual void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);

        private:
            virtual void file_header(
                Sample & sample);

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & sample);

        private:
            std::string video_format_;
            std::string audio_format_;
        };

        PPBOX_REGISTER_MUXER("raw", RawMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RAW_RAW_MUXER_H_
