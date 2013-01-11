// PackageJoinTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/PackageJoinTransfer.h"

#include <framework/system/BytesOrder.h>

namespace ppbox
{
    namespace mux
    {

        PackageJoinTransfer::PackageJoinTransfer()
            : frame_data_size_(0)
        {
        }

        void PackageJoinTransfer::transfer(
            Sample & sample)
        {
            std::vector<ppbox::avformat::NaluBuffer> & nalus = 
                *(std::vector<ppbox::avformat::NaluBuffer> *)sample.context;

            sample.size = 0;
            helper_.nalus(nalus);
            helper_.to_packet(sample.size, sample.data);

            sample.context = NULL;
        }

    } // namespace mux
} // namespace ppbox
