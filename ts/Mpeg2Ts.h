//Mpeg2Ts.h

#ifndef      _PPBOX_MUX_TS_MPEG2TS_H_
#define      _PPBOX_MUX_TS_MPEG2TS_H_

#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/AvcConfig.h"
#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/ts/MpegTsType.h"

#include <bento4/Core/Ap4.h>
#include <bento4/Core/Ap4Types.h>

const boost::uint16_t AP4_MPEG2_TS_DEFAULT_PID_PMT         = 0x100;
const boost::uint16_t AP4_MPEG2_TS_DEFAULT_PID_AUDIO       = 0x101;
const boost::uint16_t AP4_MPEG2_TS_DEFAULT_PID_VIDEO       = 0x102;
const boost::uint16_t AP4_MPEG2_TS_DEFAULT_STREAM_ID_AUDIO = 0xc0;
const boost::uint16_t AP4_MPEG2_TS_DEFAULT_STREAM_ID_VIDEO = 0xe0;

const boost::uint8_t AP4_MPEG2_STREAM_TYPE_ISO_IEC_13818_7 = 0x0F;
const boost::uint8_t AP4_MPEG2_STREAM_TYPE_ISO_IEC_13818_3 = 0x03;
const boost::uint8_t AP4_MPEG2_STREAM_TYPE_AVC             = 0x1B;

const boost::uint32_t   AP4_MPEG2TS_PACKET_SIZE         = 188;
const boost::uint32_t   AP4_MPEG2TS_PACKET_PAYLOAD_SIZE = 184;
const boost::uint32_t   AP4_MPEG2TS_SYNC_BYTE           = 0x47;
const boost::uint32_t   AP4_MPEG2TS_PCR_ADAPTATION_SIZE = 6;

namespace ppbox
{
    namespace mux
    {

        class Stream 
            : public Transfer
        {
        public:
            Stream(AP4_UI16 pid) 
                : pid_(pid)
                , continuity_counter_(0)
                , ts_header_(new AP4_MemoryByteStream(AP4_MPEG2TS_PACKET_SIZE))
            {
            }

            virtual ~Stream()
            {
                if (ts_header_) {
                    ts_header_->Release();
                }
            }

            virtual void transfer(ppbox::demux::Sample & sample)
            {
            }

            boost::uint32_t GetPID()
            {
                return pid_;
            }

            void WritePacketHeader(
                bool payload_start, 
                boost::uint32_t & payload_size,
                boost::uint32_t & head_size, // in: spare size, out: real size
                bool with_pcr,
                boost::uint64_t pcr,
                boost::uint8_t * ptr);

        private:
            boost::uint32_t pid_;
            boost::uint32_t continuity_counter_;
            AP4_MemoryByteStream * ts_header_;

            // for serialize
            TransportPacket   transport_packet_;
            AdaptationField   adapation_field_;
            //char ts_packet_header_buffer_[AP4_MPEG2TS_PACKET_SIZE];
            boost::uint32_t   ts_packet_header_size_;
        };

    }
}

#endif // End of _PPBOX_MUX_TS_MPEG2TS_H_
