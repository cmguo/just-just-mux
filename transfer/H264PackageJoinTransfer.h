// H264PackageJoinTransfer.h

#ifndef _PPBOX_MUX_TRANSFER_H264_PACKAGE_JOIN_TRANSFER_H_
#define _PPBOX_MUX_TRANSFER_H264_PACKAGE_JOIN_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <ppbox/avformat/codec/avc/AvcNaluHelper.h>

namespace ppbox
{
    namespace mux
    {

        class H264PackageJoinTransfer
            : public Transfer
        {
        public:
            H264PackageJoinTransfer();

        public:
            virtual void transfer(
                Sample & sample);

        private:
            ppbox::avformat::AvcNaluHelper helper_;
            boost::uint32_t frame_data_size_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TRANSFER_H264_PACKAGE_JOIN_TRANSFER_H_
