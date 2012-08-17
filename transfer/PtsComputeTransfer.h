// PtsComputeTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_PTS_COMPUTE_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_PTS_COMPUTE_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"

#include <ppbox/avformat/codec/AvcSpsPpsType.h>

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

            virtual void transfer(
                ppbox::demux::Sample & sample);

        private:
            std::map<boost::uint32_t, ppbox::avformat::SeqParameterSetRbsp> spss_;
            std::map<boost::uint32_t, ppbox::avformat::PicParameterSetRbsp> ppss_;
            boost::uint64_t idr_dts_;
            boost::uint32_t frame_scale_;
            bool is_last_a_idr_;
        };
    }
}

#endif // _PPBOX_MUX_TRANSFER_PTS_COMPUTE_TRANSFER_H_
