// AvcDecode.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/decode/AvcDecode.h"
#include "ppbox/mux/AvcConfig.h"

namespace ppbox
{
    namespace mux
    {
        void AvcDecode::config(
            std::vector<boost::uint8_t> & data,
            void *& config)
        {
            avc_config_.set_buffer(&data.at(0), data.size());
            avc_config_.creat();
            config = (void*)&avc_config_;
        }
    }
}
