// M3u8Protocol.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/m3u8/M3u8Protocol.h"

#include <ostream>

std::string const M3U8_BEGIN = "#EXTM3U";
std::string const M3U8_TARGETDURATION = "#EXT-X-TARGETDURATION:";
std::string const M3U8_SEQUENCE = "#EXT-X-MEDIA-SEQUENCE:";
std::string const M3U8_EXTINF = "#EXTINF:";
std::string const M3U8_END  = "#EXT-X-ENDLIST";
std::string const M3U8_ENDLINE = "\n";

namespace ppbox
{
    namespace mux
    {

        struct String
        {
            String(
                char const * str, 
                size_t len)
                : str(str)
                , len(len)
            {
            }

            char const * str;
            size_t len;
        };

        class UrlFormator
        {
        public:
            UrlFormator(
                std::string const & format)
            {
                char const * accept_tokens = "ntd";

                assert(!format.empty());
                char const * p = format.c_str();
                while (*p) {
                    for (char const * q = p; ; *q) {
                        if (*q == '%') {
                            if (q > p) {
                                strs_.push_back(String(p, q - p));
                            }
                            ++q;
                            if (*q) {
                                if (char const * t = strchr(accept_tokens, *q)) {
                                    strs_.push_back(String(NULL, t - accept_tokens));
                                } else {
                                    strs_.push_back(String(q, 1));
                                }
                                ++q;
                            }
                            p = q;
                            break;
                        }
                    }
                }
            }

        public:
            void apply(
                std::ostream & out, 
                boost::uint64_t values[3]) const
            {
                for (size_t i = 0; i < strs_.size(); ++i) {
                    if (strs_[i].str) {
                        out.write(strs_[i].str, strs_[i].len);
                    } else {
                        out << values[strs_[i].len];
                    }
                }
            }

        private:
            std::vector<String> strs_;
        };

        bool M3u8Protocol::create(
            std::ostream & out, 
            M3u8Config const & config, 
            ppbox::data::MediaBase const & media, 
            boost::system::error_code & ec)
        {
            ppbox::data::MediaInfo info;
            if (!media.get_info(info, ec)) {
                return false;
            }
            if (info.is_live && info.delay < (config.live_delay * config.interval * 1000)) {
                ec = framework::system::logic_error::invalid_argument;
                return false;
            }
            boost::uint32_t interval = config.interval * 1000;
            if (info.is_live) {
                info.current -= info.duration;
                info.duration += info.current;
                info.duration -= info.delay;
                info.current /= interval;
                info.current *= interval;
                info.duration /= interval;
                info.duration *= interval;
                info.duration += config.live_delay * interval;
            }
            boost::uint64_t time_beg = info.is_live ? info.current : 0;
            boost::uint64_t time_end = info.duration;
            boost::uint64_t index_beg = time_beg / interval;
            boost::uint64_t index_end = time_end / interval;
            append_head(out, config.interval, index_beg);
            UrlFormator formator(config.url_format);
            append_body(out, formator, config.interval, index_beg, index_end);
            if (index_end * interval < time_end) {
                append_remnant(out, formator, config.interval, index_beg, (boost::uint32_t)(time_end - index_end * interval));
            }
            append_tail(out, !info.is_live);
            ec.clear();
            return true;
        }

        void M3u8Protocol::append_head(
            std::ostream & out, 
            boost::uint32_t interval, 
            boost::uint64_t begin)
        {
            out << M3U8_BEGIN << "\n";
            out << M3U8_TARGETDURATION << interval << "\n";
            out << M3U8_SEQUENCE << begin << "\n";
        }

        void M3u8Protocol::append_body(
            std::ostream & out, 
            UrlFormator const & format, 
            boost::uint32_t interval, 
            boost::uint64_t begin, 
            boost::uint64_t end)
        {
            boost::uint64_t values[3] = {begin, begin * interval, interval * 1000};
            for (values[0] = begin; values[0] < end; ++values[0], values[1] += interval) {
                out << M3U8_EXTINF << interval << ",\n";
                format.apply(out, values);
            }
        }

        void M3u8Protocol::append_remnant(
            std::ostream & out, 
            UrlFormator const & format, 
            boost::uint32_t interval,
            boost::uint64_t index, 
            boost::uint32_t remnant)
        {
            boost::uint64_t values[3] = {index, index * interval, remnant};
            out << M3U8_EXTINF << (float)remnant / 1000 << ",\n";
            format.apply(out, values);
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
