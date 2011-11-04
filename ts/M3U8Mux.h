// M3U8Mux.h

#ifndef _PPBOX_MUX_TS_M3U8_MUX_H_
#define _PPBOX_MUX_TS_M3U8_MUX_H_

#include "ppbox/mux/ts/TsMux.h"
#include "ppbox/mux/filter/SegmentFilter.h"
#include "ppbox/mux/ts/M3U8Protocol.h"

namespace ppbox
{
    namespace mux
    {
        class M3U8Mux
            : public TsMux
        {
        public:
            M3U8Mux()
                : begin_index_(1)
                , m3u8_protocol_(*this)
                , segment_filter_(Muxer::mediainfo())
            {
                add_filter(segment_filter_);
            }

            ~M3U8Mux()
            {
            }

        public:
            void add_stream(
                ppbox::demux::MediaInfo & mediainfo,
                std::vector<Transfer *> & transfer);

            void head_buffer(
                ppbox::demux::Sample & tag);

            boost::system::error_code seek(
                boost::uint32_t & time,
                boost::system::error_code & ec);

            MediaFileInfo & mediainfo(void);

        private:
            boost::uint32_t begin_index_;
            std::string full_path_;
            std::string m3u8_cache_;
            M3U8Protocol m3u8_protocol_;
            SegmentFilter segment_filter_;

        };
    }
}

#endif // _PPBOX_MUX_TS_M3U8_MUX_H_
