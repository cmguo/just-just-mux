// PackageJoinTransfer.h

#ifndef _PPBOX_TRANSFER_MUX_PACKAGE_JOIN_TRANSFER_H_
#define _PPBOX_TRANSFER_MUX_PACKAGE_JOIN_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

namespace ppbox
{
    namespace mux
    {

        class PackageJoinTransfer
            : public Transfer
        {
        public:
            PackageJoinTransfer();

        public:
            virtual void transfer(
                Sample & sample);

        private:
            boost::uint32_t frame_data_size_;
            std::vector<boost::uint8_t> sps_pps_data_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_TRANSFER_MUX_PACKAGE_JOIN_TRANSFER_H_
