// MuxerType.h

#ifndef   _PPBOX_MUX_MUXER_TYPE_H_
#define   _PPBOX_MUX_MUXER_TYPE_H_

namespace ppbox
{
    namespace mux
    {
        struct MuxerType
        {
            enum Enum
            {
                NONE,
                FLV,
                TS,
                RTPES,
                RTPTS,
                m3u8,
            };
        };
    }
}

#endif // _PPBOX_MUX_MUXER_TYPE_H_
