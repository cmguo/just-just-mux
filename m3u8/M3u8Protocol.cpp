// M3u8Protocol.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/m3u8/M3u8Protocol.h"
#include "ppbox/mux/m3u8/UrlFormator.h"

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

        /*
            M3U8 Э��ע���
            1��EXT-X-MEDIA-SEQUENCE �� 1 ��ʼ
            2��������ͷҪ����ʱ��ֵ��д EXTINF ʱ��������ȡ��
            2��ʱ��ֵ�������������Ǹ���������ʹ������ͷҲ����ȡ��
        */

        bool M3u8Protocol::create(
            std::ostream & out, 
            M3u8Config const & config, 
            MediaInfo const & info1, 
            boost::system::error_code & ec)
        {
            MediaInfo info = info1;
            if (info.type == MediaInfo::live && info.delay < (config.live_delay * config.interval * 1000)) {
                ec = framework::system::logic_error::invalid_argument;
                return false;
            }
            boost::uint32_t interval = config.interval * 1000;
            if (info.type == MediaInfo::live) {
                info.current -= info.duration;
                info.duration += info.current;
                info.duration -= info.delay;
                info.current /= interval;
                info.current *= interval;
                info.duration /= interval;
                info.duration *= interval;
                info.duration += config.live_delay * interval;
            }
            boost::uint64_t time_beg = info.type == MediaInfo::live ? info.current : 0;
            boost::uint64_t time_end = info.duration;
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
