// H264PackageSplitTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/H264PackageSplitTransfer.h"

#include <ppbox/avcodec/avc/AvcCodec.h>
#include <ppbox/avcodec/avc/AvcConfig.h>

namespace ppbox
{
    namespace mux
    {

        H264PackageSplitTransfer::H264PackageSplitTransfer()
        {
        }

        void H264PackageSplitTransfer::transfer(
            StreamInfo & info)
        {
            ppbox::avcodec::AvcConfigHelper const & avc_config = 
                ((ppbox::avcodec::AvcCodec const *)info.codec.get())->config_helper();

            helper_.nalu_length_size((boost::uint8_t)avc_config.data().lengthSizeMinusOne + 1);
        }

        void H264PackageSplitTransfer::transfer(
            Sample & sample)
        {
            helper_.from_packet(sample.size, sample.data);
            sample.context = (void *)&helper_.nalus();
        }

    } // namespace mux
} // namespace ppbox
