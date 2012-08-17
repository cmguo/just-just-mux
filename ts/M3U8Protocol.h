// M3u8Protocol.h

#ifndef   _PPBOX_MUX_TS_M3U8_PROTOCOL_H_
#define   _PPBOX_MUX_TS_M3U8_PROTOCOL_H_

#include <ppbox/common/SegmentBase.h>

#include <framework/timer/TickCounter.h>
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
        class Muxer;

        class M3U8Protocol
        {
        public:
            M3U8Protocol(Muxer & muxer);

            ~M3U8Protocol();

            std::string create(
                boost::uint32_t begin_index,
                ppbox::common::DurationInfo const & info);

            std::string create(
             boost::uint32_t begin_index,
             ppbox::common::DurationInfo const & info,
             std::string full_path);

            std::string create(
                boost::uint32_t begin,
                boost::uint32_t end,
                bool with_end_list);

            boost::uint32_t segment_duration(void);

        private:
            boost::uint32_t seg_duration_;
            std::string full_path_;
            std::string context_;
            framework::timer::TickCounter tc_;
            boost::uint32_t back_seek_time_;
        };
    }
}

#endif
