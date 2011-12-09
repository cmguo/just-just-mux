// AvcDecode.h

#ifndef   _PPBOX_MUX_AVC_DECODE_
#define   _PPBOX_MUX_AVC_DECODE_

#include "ppbox/mux/decode/Decode.h"
#include "ppbox/mux/AvcConfig.h"

namespace ppbox
{
    namespace mux
    {
        class AvcDecode
            : public Decode
        {
        public:
            AvcDecode()
            {
            }

            ~AvcDecode()
            {
            }

            virtual void config(
                std::vector<boost::uint8_t> & data,
                void *& config);

        private:
            AvcConfig avc_config_;
        };
    }
}

#endif // _PPBOX_MUX_AVC_DECODE_
