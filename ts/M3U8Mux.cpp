// M3U8Mux.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/M3U8Mux.h"
#include "ppbox/mux/ts/M3U8Protocol.h"

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        void M3U8Mux::add_stream(
            MediaInfoEx & mediainfo,
            std::vector<Transfer *> & transfer)
        {
            TsMux::add_stream(mediainfo, transfer);
        }

        void M3U8Mux::file_header(
            ppbox::demux::Sample & tag)
        {
            TsMux::file_header(tag);
        }

        void M3U8Mux::stream_header(
            boost::uint32_t index, 
            ppbox::demux::Sample & tag)
        {
            TsMux::stream_header(index, tag);
        }

        error_code M3U8Mux::seek(
            boost::uint32_t & segment_index,
            error_code & ec)
        {
            ec.clear();
            if (segment_index >= 1) {
                if (begin_index_ + 1 != segment_index) {
                    boost::uint32_t seek_time = (segment_index - 1) * m3u8_protocol_.segment_duration();
                    seek_time *= 1000;
                    Muxer::seek(seek_time, ec);
                    if (!ec || ec == boost::asio::error::would_block) {
                        segment_filter_.segment_end_time() 
                            = segment_index * m3u8_protocol_.segment_duration() * 1000000;
                    } else {
                        return ec;
                    }
                } else {
                    segment_filter_.segment_end_time() 
                        = segment_index * m3u8_protocol_.segment_duration() * 1000000;
                }
                begin_index_ = segment_index;
            }
            return ec;
        }

        MediaFileInfo & M3U8Mux::mediainfo(void)
        {
            m3u8_cache_ = m3u8_protocol_.create(begin_index_, Muxer::mediainfo().duration);
            Muxer::mediainfo().attachment = (void*)&m3u8_cache_;
            return Muxer::mediainfo();
        }

    }
}
