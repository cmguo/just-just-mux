// PackageJoinTransfer.h

#ifndef _PPBOX_TRANSFER_MUX_PACKAGE_JOIN_TRANSFER_H_
#define _PPBOX_TRANSFER_MUX_PACKAGE_JOIN_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avformat/codec/avc/AvcNaluHelper.h>

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
            ppbox::avformat::AvcNaluHelper helper_;
            boost::uint32_t frame_data_size_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_TRANSFER_MUX_PACKAGE_JOIN_TRANSFER_H_
