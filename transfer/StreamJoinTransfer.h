// StreamJoinTransfer.h

#ifndef   _PPBOX_MUX_STREAM_TRANSFER_H_
#define   _PPBOX_MUX_STREAM_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"

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

            virtual void transfer(
                ppbox::mux::MediaInfoEx & media);

            virtual void transfer(
                ppbox::demux::Sample & sample);

        private:
            std::vector<boost::uint8_t> nalu_start_code_;
            std::vector<boost::uint8_t> access_unit_delimiter_;
            std::vector<boost::uint8_t> sps_pps_;
            boost::uint32_t sample_description_index_;
        };
    }
}
#endif
