// RtpTsMux.h

#ifndef   _PPBOX_MUX_RTP_TS_MUX_H_
#define   _PPBOX_MUX_RTP_TS_MUX_H_

#include "ppbox/mux/ts/TsMux.h"
#include "ppbox/mux/rtp/RtpPacket.h"

#include <ppbox/demux/base/DemuxerBase.h>

namespace ppbox
{
    namespace mux
    {
        class RtpTsTransfer;

        class RtpTsMux
            : public TsMux
        {
        public:
            RtpTsMux()
                : map_id_(33)
                , rtp_ts_transfer_(NULL)
            {
            }

            ~RtpTsMux();

        public:
            void add_stream(
                ppbox::demux::MediaInfo & mediainfo,
                std::vector<Transfer *> & transfer);

            void head_buffer(ppbox::demux::Sample & tag);

            boost::system::error_code seek(
                boost::uint32_t & time,
                boost::system::error_code & ec);

        private:
            boost::uint32_t map_id_;
            RtpTsTransfer * rtp_ts_transfer_;
        };
    }
}

#endif // End _PPBOX_MUX_RTP_TS_MUX_H_
