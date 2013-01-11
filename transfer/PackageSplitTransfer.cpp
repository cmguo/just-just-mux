// PackageSplitTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/PackageSplitTransfer.h"

#include <ppbox/avformat/codec/avc/AvcCodec.h>
#include <ppbox/avformat/codec/avc/AvcConfig.h>

namespace ppbox
{
    namespace mux
    {

        PackageSplitTransfer::PackageSplitTransfer()
        {
        }

        void PackageSplitTransfer::transfer(
            StreamInfo & info)
        {
            ppbox::avformat::AvcConfigHelper const & avc_config = 
                ((ppbox::avformat::AvcCodec const *)info.codec.get())->config_helper();

            helper_.nalu_length_size((boost::uint8_t)avc_config.data().lengthSizeMinusOne + 1);
        }

        void PackageSplitTransfer::transfer(
            Sample & sample)
        {
            helper_.from_packet(sample.size, sample.data);
            sample.context = (void *)&helper_.nalus();
        }

    } // namespace mux
} // namespace ppbox
