// RtpEsVideoTransfer.h

#ifndef   _PPBOX_MUX_RTP_ES_VIDEO_TRANSFER_H_
#define   _PPBOX_MUX_RTP_ES_VIDEO_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"
#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/detail/BitsReader.h"

namespace ppbox
{
    namespace mux
    {
        class RtpEsVideoTransfer
            : public RtpTransfer
        {
        public:
            RtpEsVideoTransfer(
                Muxer & muxer,
                boost::uint8_t type);

            ~RtpEsVideoTransfer();

            virtual void transfer(ppbox::demux::Sample & sample);

            virtual void get_rtp_info(ppbox::demux::MediaInfo & info);

            virtual void on_seek(boost::uint32_t time, boost::uint32_t play_time);

        private:
            boost::uint8_t prefix_[3][2];
            std::vector<MyBuffersPosition> positions_;

        };
    }
}
#endif // _PPBOX_MUX_RTP_ES_VIDEO_TRANSFER_H_
