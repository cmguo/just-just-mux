// MuxerBase.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"

#include <ppbox/demux/Demuxer.h>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {
        MediaFileInfo & MuxerBase::media_info(void)
        {
            return media_file_info_;
        }

        boost::uint64_t & MuxerBase::current_time(void)
        {
            return current_tag_time_;
        }

        boost::uint32_t & MuxerBase::video_track_index(void)
        {
            return video_index_;
        }

        boost::uint32_t & MuxerBase::audio_track_index(void)
        {
            return audio_index_;
        }

        demux::Demuxer *& MuxerBase::demuxer(void)
        {
            return demuxer_;
        }

        error_code MuxerBase::get_buffer_time(
            boost::uint32_t & buffer_time,
            error_code & ec)
        {
            boost::uint32_t cur = demuxer_->get_cur_time(ec);
            if (!ec) {
                error_code ec_buf;
                boost::uint32_t end = demuxer_->get_end_time(ec, ec_buf);
                if (!ec) {
                    buffer_time = end > cur ? (end-cur) : 0;
                    if (ec_buf == boost::asio::error::eof) {
                        ec = ec_buf;
                    }
                }
            }
            return ec;
        }
    }
}