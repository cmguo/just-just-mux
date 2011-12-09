// RtpTsTransfer.h

#ifndef   _PPBOX_MUX_RTP_TS_TRANSFER_H_
#define   _PPBOX_MUX_RTP_TS_TRANSFER_H_

#include "ppbox/mux/rtp/RtpTransfer.h"
#include "ppbox/mux/Muxer.h"

namespace ppbox
{
    namespace mux
    {
        class RtpTsTransfer
            : public RtpTransfer
        {
        public:
            RtpTsTransfer(
                Muxer & muxer,
                boost::uint8_t type);

            ~RtpTsTransfer();

            virtual void transfer(ppbox::demux::Sample & sample);

            virtual void get_rtp_info(MediaInfoEx & info);

            virtual void on_seek(boost::uint32_t time, boost::uint32_t play_time);

            void header_rtp_packet(ppbox::demux::Sample & tag);
        };
    }
}

#endif // _PPBOX_MUX_RTP_TS_TRANSFER_H_
