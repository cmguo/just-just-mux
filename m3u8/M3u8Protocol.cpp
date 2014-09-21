// M3u8Protocol.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/m3u8/M3u8Protocol.h"
#include "ppbox/mux/m3u8/UrlFormator.h"

namespace ppbox
{
    namespace mux
    {

        /*
            M3U8 协议注意点
            1、EXT-X-MEDIA-SEQUENCE 从 1 开始
            2、最后的零头要按照时间值填写 EXTINF 时长，向上取整
            2、时间值是整数，不能是浮点数，即使最后的零头也必须取整
        */

        bool M3u8Protocol::create(
            std::ostream & out, 
            M3u8Config const & config, 
            MediaInfo const & info, 
            boost::system::error_code & ec)
        {
            if (info.type == MediaInfo::live && info.delay < (config.live_delay * config.interval * 1000)) {
                ec = framework::system::logic_error::invalid_argument;
                return false;
            }
            boost::uint32_t interval = config.interval * 1000;
            boost::uint64_t time_beg = 0;
            boost::uint64_t time_end = info.duration;
            if (info.type == MediaInfo::live) {
                time_beg = info.current - info.shift;
                time_end = info.current - info.delay + config.live_delay * interval;
                time_beg /= interval;
                time_beg *= interval;
                time_end /= interval;
                time_end *= interval;
            }
            boost::uint64_t index_beg = time_beg / interval;
            boost::uint64_t index_end = time_end / interval;
            append_head(out, config.interval, index_beg);
            UrlFormator formator(config.url_format);
            append_body(out, formator, config.interval, index_beg, index_end);
            if (index_end * interval < time_end) {
                append_remnant(out, formator, config.interval, index_end, (boost::uint32_t)(time_end - index_end * interval));
            }
            append_tail(out, !info.type == MediaInfo::live);
            ec.clear();
            return true;
        }

        char const * const M3U8_BEGIN = "#EXTM3U";
        char const * const M3U8_TARGETDURATION = "#EXT-X-TARGETDURATION:";
        char const * const M3U8_SEQUENCE = "#EXT-X-MEDIA-SEQUENCE:";
        char const * const M3U8_EXTINF = "#EXTINF:";
        char const * const M3U8_END  = "#EXT-X-ENDLIST";

        void M3u8Protocol::append_head(
            std::ostream & out, 
            boost::uint32_t interval, 
            boost::uint64_t begin)
        {
            out << M3U8_BEGIN << "\n";
            out << M3U8_TARGETDURATION << interval << "\n";
            out << M3U8_SEQUENCE << begin + 1 << "\n";
        }

        void M3u8Protocol::append_body(
            std::ostream & out, 
            UrlFormator const & formator, 
            boost::uint32_t interval, 
            boost::uint64_t begin, 
            boost::uint64_t end)
        {
            boost::uint64_t values[3] = {begin, begin * interval, interval * 1000};
            for (values[0] = begin; values[0] < end; ++values[0], values[1] += interval) {
                out << M3U8_EXTINF << interval << ",\n";
                out << formator(values) << "\n";
            }
        }

        void M3u8Protocol::append_remnant(
            std::ostream & out, 
            UrlFormator const & formator, 
            boost::uint32_t interval,
            boost::uint64_t index, 
            boost::uint32_t remnant)
        {
            boost::uint64_t values[3] = {index, index * interval, remnant};
            out << M3U8_EXTINF << (remnant + 999) / 1000 << ",\n";
            out << formator(values) << "\n";
        }

        void M3u8Protocol::append_tail(
            std::ostream & out, 
            bool is_end)
        {
            if (is_end) {
                out << M3U8_END << "\n";
            }
        }

    } // namespace mux
} // namespace ppbox
