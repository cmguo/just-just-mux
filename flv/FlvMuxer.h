// FlvMuxer.h

#ifndef _PPBOX_MUX_FLV_MUXER_H_
#define _PPBOX_MUX_FLV_MUXER_H_

#include "ppbox/mux/MuxerBase.h"

#include <ppbox/avformat/flv/FlvTagType.h>

namespace ppbox
{
    namespace mux
    {

        class FlvMuxer
            : public MuxerBase
        {
        public:
            FlvMuxer();

            ~FlvMuxer();

        public:
            void add_stream(
                MediaInfoEx & mediainfo);

            void file_header(
                ppbox::demux::Sample & tag);

            void stream_header(
                boost::uint32_t index, 
                ppbox::demux::Sample & tag);

        private:
            ppbox::avformat::FlvHeader flv_header_;
            ppbox::avformat::FlvTagHeader flv_tag_header_;
            ppbox::avformat::FlvAudioTagHeader flv_audio_tag_header_;
            ppbox::avformat::FlvVideoTagHeader flv_video_tag_header_;
            char audio_header_buffer_[13];
            char video_header_buffer_[16];
            char flv_file_header_buffer_[13];
            boost::uint32_t audio_header_size_;
            boost::uint32_t video_header_size_;
        };

        PPBOX_REGISTER_MUXER(flv, FlvMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FLV_MUXER_H_
