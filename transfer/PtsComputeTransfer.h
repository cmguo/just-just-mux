// PtsComputeTransfer.h

#ifndef   _PPBOX_MUX_PTS_COMPUTE_TRANSFER_H_
#define   _PPBOX_MUX_PTS_COMPUTE_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/tinyvlc.h"

namespace ppbox
{
    namespace mux
    {
        class PtsComputeTransfer
            : public Transfer
        {
        public:
            PtsComputeTransfer()
                : idr_dts_(0)
                , frame_scale_(0)
                , is_last_a_idr_(false)
            {
            }

            ~PtsComputeTransfer()
            {
            }

            void transfer(ppbox::demux::Sample & sample);

        private:
            boost::uint64_t idr_dts_;
            boost::uint32_t frame_scale_;
            bool is_last_a_idr_;
            NaluParser nalu_parser_;
        };
    }
}
#endif
