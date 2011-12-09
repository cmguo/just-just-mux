// Decode.h

#ifndef   _PPBOX_MUX_DECODE_
#define   _PPBOX_MUX_DECODE_

#include <boost/asio/buffer.hpp>

namespace ppbox
{
    namespace mux
    {
        class Decode
        {
        public:
            Decode()
            {
            }

            virtual ~Decode()
            {
            }

            virtual void config(
                std::vector<boost::uint8_t> & data,
                void *& config) = 0;

        };
    }
}

#endif // _PPBOX_MUX_DECODE_
