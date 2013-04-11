// M3U8Mux.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/m3u8/M3u8Muxer.h"
#include "ppbox/mux/m3u8/M3u8Protocol.h"
#include "ppbox/mux/filter/SegmentFilter.h"

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        M3u8Muxer::M3u8Muxer()
            : next_index_(0)
            , segment_filter_(NULL)
        {
            config().register_module("M3u8Protocol")
                << CONFIG_PARAM_NAME_RDWR("interval", m3u8_config_.interval)
                << CONFIG_PARAM_NAME_RDWR("live_delay", m3u8_config_.live_delay)
                << CONFIG_PARAM_NAME_RDWR("url_format", m3u8_config_.url_format);

        }

        M3u8Muxer::~M3u8Muxer()
        {
        }

        void M3u8Muxer::do_open(
            MediaInfo & info)
        {
            segment_filter_ = new SegmentFilter;
            add_filter(segment_filter_);
        }

        void M3u8Muxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            TsMuxer::add_stream(info, transfers);
        }

        void M3u8Muxer::file_header(
            Sample & sample)
        {
            TsMuxer::file_header(sample);
        }

        void M3u8Muxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
            TsMuxer::stream_header(index, sample);
        }

        void M3u8Muxer::do_close()
        {
            delete segment_filter_;
            segment_filter_ = NULL;
        }

        bool M3u8Muxer::time_seek(
            boost::uint64_t & time,
            error_code & ec)
        {
            ec.clear();
            boost::uint64_t index = time;
            if (next_index_ != index || !segment_filter_->is_sequence()) {
                time = index * m3u8_config_.interval * 1000;
                TsMuxer::time_seek(time, ec);
            } else {
                reset_header();
            }
            if (!ec || ec == boost::asio::error::would_block) {
                next_index_ = index + 1;
                segment_filter_->set_end_time(m3u8_config_.interval * next_index_ * 1000000);
            }
            return !ec;
        }

        void M3u8Muxer::media_info(
            MediaInfo & info) const
        {
            TsMuxer::media_info(info);
            if (info.type == MediaInfo::live || m3u8_cache_.empty()) {
                if (info.type == MediaInfo::live && info.delay == 0) {
                    info.delay = m3u8_config_.live_delay * m3u8_config_.interval * 1000;
                    info.duration = info.delay;
                    info.current += info.duration;
                }
                boost::system::error_code ec;
                std::ostringstream oss;
                M3u8Protocol::create(oss, m3u8_config_, info, ec);
                assert(!ec);
                m3u8_cache_ = oss.str();
            }
            info.format = "ts";
            info.format_data = m3u8_cache_;
        }

    } // namespace mux
} // namespace ppbox
