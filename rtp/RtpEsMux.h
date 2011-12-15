// RtpEsMux.h
#ifndef   _PPBOX_MUX_RTP_ES_MUX_H_
#define   _PPBOX_MUX_RTP_ES_MUX_H_

#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/rtp/RtpPacket.h"

#include <ppbox/demux/base/DemuxerBase.h>

namespace ppbox
{
    namespace mux
    {
        class RtpEsMux
            : public Muxer
        {
        public:
            RtpEsMux()
                : audio_map_id_(97)
                , video_map_id_(96)
            {
            }

            ~RtpEsMux()
            {
            }

        public:
            void add_stream(
                MediaInfoEx & mediainfo);

            void file_header(
                ppbox::demux::Sample & tag);

            void stream_header(
                boost::uint32_t index, 
                ppbox::demux::Sample & tag);

            boost::system::error_code seek(
                boost::uint32_t & time,
                boost::system::error_code & ec);

        private:
            boost::uint32_t audio_map_id_;
            boost::uint32_t video_map_id_;
            std::vector<RtpTransfer *> transfers_;

        };
    }
}

#endif // End _PPBOX_MUX_FLV_FLVMUX_H_
