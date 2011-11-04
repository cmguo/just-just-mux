//ByteOrder.h

#ifndef      _PPBOX_MUX_BYTE_ORDER_
#define      _PPBOX_MUX_BYTE_ORDER_

namespace ppbox
{
    namespace mux
    {

        /*return 1 : little-endian, return 0:big-endian*/
        static bool endian(void)
        {
            union
            {
                boost::uint32_t a;
                boost::uint8_t  b;
            } c;
            c.a = 1;
            return (c.b == 1);
        }

        static inline void host_uint24_to_big_endian(boost::uint32_t v, boost::uint8_t out[3])
        {
            if (endian()) {
                out[0] = (v&0xff0000) >> 16;
                out[1] = (v&0x00ff00) >> 8;
                out[2] = v&0x0000ff;
            } else {
                boost::uint8_t * p = (boost::uint8_t *)&v;
                memcpy(out, p+1, 3);
            }
        }
    } // namespace mux
} // namespace ppbox

#endif // End of _PPBOX_MUX_BYTE_ORDER_
