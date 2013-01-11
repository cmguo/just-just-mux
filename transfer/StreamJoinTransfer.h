// StreamJoinTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_STREAM_JOIN_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_STREAM_JOIN_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avformat/codec/avc/AvcNaluHelper.h>

namespace ppbox
{
    namespace mux
    {

        class StreamJoinTransfer
            : public Transfer
        {
        public:
            StreamJoinTransfer();

            ~StreamJoinTransfer();

        public:
            virtual void transfer(
                ppbox::mux::StreamInfo & media);

            virtual void transfer(
                Sample & sample);

        private:
            ppbox::avformat::AvcNaluHelper helper_;
            std::vector<boost::uint8_t> access_unit_delimiter_;
            std::vector<boost::uint8_t> sps_pps_;
            boost::uint32_t sample_description_index_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_STREAM_JOIN_TRANSFER_H_
