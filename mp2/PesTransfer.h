// PesTransfer.h

#ifndef _PPBOX_MUX_MP2_PES_TRANSFER_H_
#define _PPBOX_MUX_MP2_PES_TRANSFER_H_

#include "ppbox/mux/mp2/TsTransfer.h"

namespace ppbox
{
    namespace mux
    {

        class PesTransfer
            : public TsTransfer
        {
        public:
            PesTransfer(
                boost::uint32_t index);

            ~PesTransfer();

        private:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

        private:
            boost::uint8_t stream_id_;
            bool with_dts_;
            boost::uint8_t pes_heaher_buffer_[20];
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MP2_PES_TRANSFER_H_
