// RtpMuxer.h

#ifndef _PPBOX_MUX_RTP_RTP_MUXER_H_
#define _PPBOX_MUX_RTP_RTP_MUXER_H_

#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/rtp/RtpPacket.h"

namespace ppbox
{
    namespace mux
    {

        class RtpTransfer;

        class RtpMuxer
            : public MuxerBase
        {
        public:
            RtpMuxer();

            RtpMuxer(
                MuxerBase * base);

            ~RtpMuxer();

        public:
            virtual bool setup(
                boost::uint32_t index, 
                boost::system::error_code & ec);

        public:
            virtual void media_info(
                MediaInfo & info) const;

            virtual void stream_status(
                StreamStatus & info) const;

        public:
            void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);

            void file_header(
                Sample & sample);

            void stream_header(
                boost::uint32_t index, 
                Sample & sample);

        protected:
           void add_rtp_transfer(
               RtpTransfer * rtp_transfer);

        private:
            MuxerBase * base_;
            std::vector<RtpTransfer *> rtp_transfers_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_MUXER_H_
