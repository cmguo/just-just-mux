// FlvMux.h
#ifndef   _PPBOX_MUX_FLV_FLVMUX_H_
#define   _PPBOX_MUX_FLV_FLVMUX_H_

#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/flv/FlvMetadata.h"

namespace ppbox
{
    namespace mux
    {

        class FlvMux
            : public Muxer
        {
        public:
            FlvMux();

            ~FlvMux();

        public:
            void add_stream(
                ppbox::demux::MediaInfo & mediainfo,
                std::vector<Transfer *> & transfer);

            void head_buffer(
                ppbox::demux::Sample & tag);

        private:
            FlvHeader flvheader_;
            FlvTag    flvtag_;
            std::vector<boost::uint8_t> file_head_buffer_;
        };
    }
}

#endif // End _PPBOX_MUX_FLV_FLVMUX_H_
