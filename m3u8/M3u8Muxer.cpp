// M3U8Mux.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/m3u8/M3u8Muxer.h"
#include "ppbox/mux/m3u8/M3u8Protocol.h"
#include "ppbox/demux/base/SegmentDemuxer.h"

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        M3u8Muxer::M3u8Muxer()
            : next_index_(0)
        {
            add_filter(segment_filter_);
        }

        M3u8Muxer::~M3u8Muxer()
        {
        }

        void M3u8Muxer::add_stream(
            StreamInfo & info)
        {
            TsMuxer::add_stream(info);
        }

        void M3u8Muxer::file_header(
            Sample & tag)
        {
            TsMuxer::file_header(tag);
        }

        void M3u8Muxer::stream_header(
            boost::uint32_t index, 
            Sample & tag)
        {
            TsMuxer::stream_header(index, tag);
        }

        bool M3u8Muxer::time_seek(
            boost::uint64_t & time,
            error_code & ec)
        {
            ec.clear();
            boost::uint64_t index = time;
            if (next_index_ != index || !segment_filter_.is_sequence()) {
                segment_filter_.reset();
                time = index * m3u8_config_.interval * 1000;
                TsMuxer::time_seek(index, ec);
            }
            if (!ec) {
                next_index_ = index + 1;
                segment_filter_.set_end_time(m3u8_config_.interval * next_index_ * 1000000);
            }
            return !ec;
        }

        void M3u8Muxer::media_info(
            MediaInfo & info)
        {
            TsMuxer::media_info(info);
            if (info.is_live || m3u8_cache_.empty()) {
                boost::system::error_code ec;
                std::ostringstream oss;
                M3u8Protocol::create(oss, m3u8_config_, info, ec);
                assert(!ec);
                m3u8_cache_ = oss.str();
            }
            info.format = "m3u8";
            info.format_data = m3u8_cache_;
        }

    } // namespace mux
} // namespace ppbox
