// PackageJoinTransfer.h

#ifndef   _PPBOX_MUX_PACKAGE_TRANSFER_H_
#define   _PPBOX_MUX_PACKAGE_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"

namespace ppbox
{
    namespace mux
    {
        class PackageJoinTransfer
            : public Transfer
        {
        public:
            PackageJoinTransfer()
                : frame_data_size_(0)
            {
            }

            ~PackageJoinTransfer()
            {
            }

            virtual void transfer(ppbox::demux::Sample & sample);

        private:
            boost::uint32_t frame_data_size_;
            std::vector<boost::uint8_t> sps_pps_data_;
        };
    }
}
#endif
