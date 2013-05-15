// H264PackageJoinTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/H264PackageJoinTransfer.h"

#include <framework/system/BytesOrder.h>

namespace ppbox
{
    namespace mux
    {

        H264PackageJoinTransfer::H264PackageJoinTransfer()
            : frame_data_size_(0)
        {
        }

        void H264PackageJoinTransfer::transfer(
            Sample & sample)
        {
            std::vector<ppbox::avcodec::NaluBuffer> & nalus = 
                *(std::vector<ppbox::avcodec::NaluBuffer> *)sample.context;

            sample.size = 0;
            helper_.nalus(nalus);
            helper_.to_packet(sample.size, sample.data);

            sample.context = NULL;
        }

    } // namespace mux
} // namespace ppbox
