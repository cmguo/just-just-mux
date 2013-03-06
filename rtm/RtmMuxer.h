// RtmMuxer.h

#ifndef _PPBOX_MUX_RTM_RTM_MUXER_H_
#define _PPBOX_MUX_RTM_RTM_MUXER_H_

#include "ppbox/mux/flv/FlvMuxer.h"

namespace ppbox
{
    namespace mux
    {

        class RtmTransfer;

        class RtmMuxer
            : public FlvMuxer
        {
        public:
            RtmMuxer();

            ~RtmMuxer();

        private:
            virtual void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);

            virtual void file_header(
                Sample & sample);

            virtual void stream_header(
                boost::uint32_t index, 
                Sample & sample);

        private:
            boost::uint8_t header_buffer_[48];
            RtmTransfer * rtm_transfer_;
        };

        PPBOX_REGISTER_MUXER("rtm", RtmMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_RTM_RTM_MUXER_H_
