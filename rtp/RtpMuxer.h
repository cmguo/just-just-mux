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
            virtual boost::system::error_code get_sdp(
                std::string & sdp_out, 
                boost::system::error_code & ec);

            virtual boost::system::error_code setup(
                boost::uint32_t index, 
                std::string & setup_out, 
                boost::system::error_code & ec);

            virtual boost::system::error_code get_rtp_info(
                std::string & rtp_info_out, 
                boost::uint32_t & seek_time,
                boost::system::error_code & ec);

        public:
            void add_stream(
                StreamInfo & info);

            void file_header(
                Sample & tag);

            void stream_header(
                boost::uint32_t index, 
                Sample & tag);

        protected:
           void add_transfer(
               RtpTransfer * rtp_transfer);

        private:
            MuxerBase * base_;
            std::vector<RtpTransfer *> rtp_transfers_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTP_RTP_MUXER_H_
