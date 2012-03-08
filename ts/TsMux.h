// TsMux.h

#ifndef _PPBOX_MUX_TS_TS_MUX_H_
#define _PPBOX_MUX_TS_TS_MUX_H_

#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/ts/Mpeg2Ts.h"

namespace ppbox
{
    namespace mux
    {
        class TsMux
            : public Muxer
        {
        public:
            TsMux();

            virtual ~TsMux();

        public:
            void add_stream(
                MediaInfoEx & mediainfo);

            void file_header(
                ppbox::demux::Sample & tag);

            void stream_header(
                boost::uint32_t index, 
                ppbox::demux::Sample & tag);

        private:
            void WritePAT(boost::uint8_t * ptr);
            void WritePMT(boost::uint8_t * ptr);

        private:
            Stream * pat_;
            Stream * pmt_;
            bool has_audio_;
            bool has_video_;
            boost::uint16_t audio_pid_;
            boost::uint16_t video_pid_;
            boost::uint16_t audio_stream_id_;
            boost::uint16_t video_stream_id_;
            boost::uint8_t audio_stream_type_;
            boost::uint8_t video_stream_type_;
            // buffer
            boost::uint8_t header_[512];
        };
    }
}

#endif // _PPBOX_MUX_TS_TS_MUX_H_
