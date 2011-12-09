// FlvMux.h
#ifndef   _PPBOX_MUX_FLV_FLVMUX_H_
#define   _PPBOX_MUX_FLV_FLVMUX_H_

#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"

#include <ppbox/demux/flv/FlvTagType.h>
//#include "ppbox/mux/flv/FlvMetadata.h"

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
                MediaInfoEx & mediainfo,
                std::vector<Transfer *> & transfer);

            void file_header(
                ppbox::demux::Sample & tag);

            void stream_header(
                boost::uint32_t index, 
                ppbox::demux::Sample & tag);

        private:
            ppbox::demux::FlvHeader flv_header_;
            ppbox::demux::FlvTagHeader flv_tag_header_;
            ppbox::demux::FlvAudioTagHeader flv_audio_tag_header_;
            ppbox::demux::FlvVideoTagHeader flv_video_tag_header_;
            char audio_header_buffer_[13];
            char video_header_buffer_[16];
            char flv_file_header_buffer_[13];
            boost::uint32_t audio_header_size_;
            boost::uint32_t video_header_size_;
        };
    }
}

#endif // End _PPBOX_MUX_FLV_FLVMUX_H_
