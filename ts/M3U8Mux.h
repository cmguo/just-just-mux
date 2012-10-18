// M3U8Mux.h

#ifndef _PPBOX_MUX_TS_M3U8_MUX_H_
#define _PPBOX_MUX_TS_M3U8_MUX_H_

#include "ppbox/mux/ts/TsMuxer.h"
#include "ppbox/mux/filter/SegmentFilter.h"
#include "ppbox/mux/m3u8/M3u8Protocol.h"

namespace ppbox
{
    namespace mux
    {
        class M3U8Mux
            : public TsMuxer
        {
        public:
            M3U8Mux();

            ~M3U8Mux();

        public:
            void add_stream(
                MediaInfoEx & mediainfo);

            void file_header(
                ppbox::demux::Sample & tag);

            void stream_header(
                boost::uint32_t index, 
                ppbox::demux::Sample & tag);

            boost::system::error_code seek(
                boost::uint32_t & time,
                boost::system::error_code & ec);

            MediaFileInfo & mediainfo(void);

        private:
            boost::uint32_t begin_index_;
            boost::uint32_t old_index_;
            std::string full_path_;
            std::string m3u8_cache_;
            M3u8Protocol m3u8_protocol_;
            SegmentFilter segment_filter_;

        };
    }
}

#endif // _PPBOX_MUX_TS_M3U8_MUX_H_
