// PesTransfer.h

#ifndef _JUST_MUX_MP2_PES_TRANSFER_H_
#define _JUST_MUX_MP2_PES_TRANSFER_H_

#include "just/mux/mp2/TsTransfer.h"

namespace just
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
} // namespace just

#endif // _JUST_MUX_MP2_PES_TRANSFER_H_
