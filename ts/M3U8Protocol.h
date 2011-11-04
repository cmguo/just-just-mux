// M3u8Protocol.h

#ifndef   _PPBOX_MUX_TS_M3U8_PROTOCOL_H_
#define   _PPBOX_MUX_TS_M3U8_PROTOCOL_H_

#include "ppbox/mux/Muxer.h"

#include <framework/string/Format.h>
#include <sstream>
#include <iostream>

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
        class M3U8Protocol
        {
        public:
            M3U8Protocol(Muxer & muxer)
                : seg_duration_(10)
                , full_path_("")
            {
                muxer.Config().register_module("M3U8")
                    << CONFIG_PARAM_NAME_RDWR("segment_duration", seg_duration_)
                    << CONFIG_PARAM_NAME_RDWR("full_path", full_path_);
            }

            ~M3U8Protocol()
            {
            }

            std::string create(boost::uint32_t begin_index, boost::uint32_t duration)
            {
                std::string result;
                boost::uint32_t lines = 0;
                if (duration > 0) {
                    lines = boost::uint32_t(duration / (seg_duration_ * 1000));
                } else {
                    lines = 5;
                }

                std::string line = M3U8_BEGIN;
                result = line + M3U8_ENDLINE;
                line = M3U8_TARGETDURATION + framework::string::format(seg_duration_);
                result+= line; result += M3U8_ENDLINE;
                line = M3U8_SEQUENCE + framework::string::format(begin_index);
                result+= line; result += M3U8_ENDLINE;
                for (boost::uint32_t i = 0; i < lines; ++i) {
                    std::string segment_line = M3U8_EXTINF + framework::string::format(seg_duration_);
                    segment_line += ",";
                    result+= segment_line; result += M3U8_ENDLINE;
                    segment_line = "";
                    if (!full_path_.empty()) {
                        segment_line += "http://" + full_path_;
                    }
                    last_segment_index_ = i+begin_index;
                    segment_line += std::string("/") + framework::string::format(last_segment_index_) + std::string(".ts");
                    result+= segment_line; result += M3U8_ENDLINE;
                }

                if (duration > 0) {
                    line = M3U8_END;
                    result+= line; result += M3U8_ENDLINE;
                }
                return result;
            }

             std::string create(
                 boost::uint32_t begin_index,
                 boost::uint32_t duration,
                 std::string full_path)
             {
                 full_path_ = full_path;
                 return create(begin_index, duration);
             }

            boost::uint32_t segment_duration(void)
            {
                return seg_duration_;
            }

        private:
            boost::uint32_t seg_duration_;
            std::string full_path_;
            boost::uint32_t last_segment_index_;
        };
    }
}

#endif
