// PtsComputeTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_PTS_COMPUTE_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_PTS_COMPUTE_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avformat/codec/avc/AvcSpsPpsType.h>

namespace ppbox
{
    namespace mux
    {

        class PtsComputeTransfer
            : public Transfer
        {
        public:
            PtsComputeTransfer();

        public:
            virtual void transfer(
                Sample & sample);

        private:
            std::map<boost::uint32_t, ppbox::avformat::SeqParameterSetRbsp> spss_;
            std::map<boost::uint32_t, ppbox::avformat::PicParameterSetRbsp> ppss_;
            boost::uint64_t idr_dts_;
            boost::uint32_t frame_scale_;
            bool is_last_a_idr_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_PTS_COMPUTE_TRANSFER_H_
