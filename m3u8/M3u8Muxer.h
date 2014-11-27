// M3u8Muxer.h

#ifndef _JUST_MUX_M3U8_M3U8_MUXER_H_
#define _JUST_MUX_M3U8_M3U8_MUXER_H_

#include "just/mux/mp2/TsMuxer.h"
#include "just/mux/m3u8/M3u8Protocol.h"

namespace just
{
    namespace mux
    {

        class SegmentFilter;

        class M3u8Muxer
            : public TsMuxer
        {
        public:
            M3u8Muxer(
                boost::asio::io_service & io_svc);

            virtual ~M3u8Muxer();

        public:
            virtual bool time_seek(
                boost::uint64_t & time,
                boost::system::error_code & ec);

            virtual void media_info(
                MediaInfo & info) const;

        private:
            virtual void do_open(
                MediaInfo & info);

            virtual void add_stream(
                StreamInfo & info, 
                FilterPipe & pipe);

            virtual void file_header(
                Sample & tag);

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & tag);

            virtual void do_close();

        private:
            boost::uint64_t next_index_;
            mutable std::string m3u8_cache_;
            M3u8Config m3u8_config_;
            SegmentFilter * segment_filter_;
        };

        JUST_REGISTER_MUXER("m3u8", M3u8Muxer);

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_M3U8_M3U8_MUXER_H_
