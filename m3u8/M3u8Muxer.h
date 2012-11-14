// M3U8Mux.h

#ifndef _PPBOX_MUX_M3U8_M3U8_MUXER_H_
#define _PPBOX_MUX_M3U8_M3U8_MUXER_H_

#include "ppbox/mux/ts/TsMuxer.h"
#include "ppbox/mux/filter/SegmentFilter.h"
#include "ppbox/mux/m3u8/M3u8Protocol.h"

namespace ppbox
{
    namespace mux
    {

        class M3u8Muxer
            : public TsMuxer
        {
        public:
            M3u8Muxer();

            virtual ~M3u8Muxer();

        public:
            virtual bool time_seek(
                boost::uint64_t & time,
                boost::system::error_code & ec);

            virtual void media_info(
                MediaInfo & info) const;

        private:
            virtual void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);

            virtual void file_header(
                Sample & tag);

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & tag);

        private:
            boost::uint64_t next_index_;
            mutable std::string m3u8_cache_;
            M3u8Config m3u8_config_;
            SegmentFilter segment_filter_;
        };

        PPBOX_REGISTER_MUXER("m3u8", M3u8Muxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_M3U8_M3U8_MUXER_H_
