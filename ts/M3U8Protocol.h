// M3u8Protocol.h

#ifndef   _PPBOX_MUX_TS_M3U8_PROTOCOL_H_
#define   _PPBOX_MUX_TS_M3U8_PROTOCOL_H_

#include <ppbox/data/MediaBase.h>

#include <framework/timer/TimeCounter.h>
namespace ppbox
{
    namespace cdn
    {
        struct DurationInfo;
    }
}

namespace ppbox
{
    namespace mux
    {
        class MuxerBase;

        class M3U8Protocol
        {
        public:
            M3U8Protocol(MuxerBase & muxer);

            ~M3U8Protocol();

            std::string create(
                boost::uint32_t begin_index,
                ppbox::data::MediaInfo const & info);

            std::string create(
             boost::uint32_t begin_index,
             ppbox::data::MediaInfo const & info,
             std::string full_path);

            std::string create(
                boost::uint32_t begin,
                boost::uint32_t end,
                bool with_end_list);

            boost::uint32_t segment_duration(void);

            bool is_last_segment(boost::uint32_t index);

            boost::uint32_t last_segment_time();

        private:
            boost::uint32_t seg_duration_;
            std::string full_path_;
            std::string context_;
            framework::timer::TimeCounter tc_;
            boost::uint32_t back_seek_time_;
            boost::uint32_t last_segment_time_;
            boost::uint32_t lines_;
        };
    }
}

#endif
