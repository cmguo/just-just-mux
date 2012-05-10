// M3u8Protocol.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/M3U8Protocol.h"
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

        M3U8Protocol::M3U8Protocol(Muxer & muxer)
            : seg_duration_(10)
            , full_path_("")
            , back_seek_time_(1800)
        {
            muxer.config().register_module("M3U8")
                << CONFIG_PARAM_NAME_RDWR("segment_duration", seg_duration_)
                << CONFIG_PARAM_NAME_RDWR("full_path", full_path_)
                << CONFIG_PARAM_NAME_RDWR("back_seek_time", back_seek_time_);
        }

        M3U8Protocol::~M3U8Protocol()
        {
        }

        std::string M3U8Protocol::create(
            boost::uint32_t begin_index,
            ppbox::demux::DurationInfo const & info)
        {
            boost::uint32_t lines = 0;
            boost::uint32_t Redundancy_size = 3;
            if (info.total > 0) {
                begin_index = 1;
                lines = boost::uint32_t(info.total / (seg_duration_ * 1000));
                create(begin_index, lines, true);
            } else {
                boost::uint32_t buffer_time = info.end - info.begin;
                lines = buffer_time / seg_duration_ + Redundancy_size;
                boost::uint32_t end_index = 0;
                if (context_.empty()) {
                    begin_index = 1;
                    end_index = lines;
                    tc_.reset();
                } else {
                    begin_index = (tc_.elapsed() / (1000 * seg_duration_)) + 1;
                    end_index = lines + begin_index - 1;
                }
                create(begin_index, end_index, false);
            }
            return context_;
        }

        std::string M3U8Protocol::create(
         boost::uint32_t begin_index,
         ppbox::demux::DurationInfo const & info,
         std::string full_path)
        {
            full_path_ = full_path;
            return create(begin_index, info);
        }

        std::string M3U8Protocol::create(
            boost::uint32_t begin,
            boost::uint32_t end,
            bool with_end_list)
        {
            assert(end > begin);
            std::string line = M3U8_BEGIN;
            context_ = line + M3U8_ENDLINE;
            line = M3U8_TARGETDURATION + framework::string::format(seg_duration_);
            context_ += line; context_ += M3U8_ENDLINE;
            line = M3U8_SEQUENCE + framework::string::format(begin);
            context_+= line; context_ += M3U8_ENDLINE;
            for (boost::uint32_t i = begin; i <= end; ++i) {
                std::string segment_line = M3U8_EXTINF + framework::string::format(seg_duration_);
                segment_line += ",";
                context_+= segment_line; context_ += M3U8_ENDLINE;
                segment_line = "";
                if (!full_path_.empty()) {
                    segment_line += "http://" + full_path_;
                }
                segment_line += std::string("/") + framework::string::format(i) + std::string(".ts");
                context_+= segment_line; context_ += M3U8_ENDLINE;
            }

            if (with_end_list) {
                line = M3U8_END;
                context_+= line; context_ += M3U8_ENDLINE;
            }
            return context_;
        }

        boost::uint32_t M3U8Protocol::segment_duration(void)
        {
            return seg_duration_;
        }

    }
}
