#ifndef _PPBOX_MUX_DEFINE_H_
#define _PPBOX_MUX_DEFINE_H_

#include "ppbox/mux/MuxerBase.h"

namespace ppbox
{
    namespace demux
    {
        class Demuxer;
    }
}

namespace ppbox
{
    namespace mux
    {
        class TsMux;

        struct SegmentType
        {
            enum Enum
            {
                NONE,
                BEGAN,
                END,
            };
        };
        class TsSegmentCreater
        {
        public:
            TsSegmentCreater(boost::uint32_t seg_duration);

            ~TsSegmentCreater();

            boost::system::error_code open(
                demux::Demuxer * demuxer, boost::system::error_code & ec);

            boost::system::error_code read(
                MuxTagEx & tag,
                boost::system::error_code & ec);

            boost::uint32_t & segment_duration(void);

            void close(void);

        private:
            MuxTagEx tag_;
            boost::uint32_t segment_duration_;
            boost::uint32_t first_idr_timestamp_;
            boost::uint64_t segment_sum_;
            SegmentType::Enum segment_state_;
            TsMux * ts_mux_;
        };
    }
}

#endif
