// Mpeg2Ts.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/Mpeg2Ts.h"
using namespace ppbox::demux;
using namespace ppbox::mux;

#include <util/buffers/BufferCopy.h>
#include <util/buffers/BufferSize.h>

#include <bento4/Core/Ap4ByteStream.h>
#include <bento4/Core/Ap4Sample.h>
#include <bento4/Core/Ap4SampleDescription.h>
#include <bento4/Core/Ap4Utils.h>

#include <iostream>
#include <stdio.h>

namespace ppbox
{
    namespace mux
    {

        void Stream::WritePacketHeader(
            bool payload_start, 
            boost::uint32_t & payload_size,
            bool with_pcr,
            boost::uint64_t pcr,
            std::vector<boost::uint8_t> & output)
        {
            ts_header_->Seek(0);
            boost::uint8_t header[4];
            header[0] = AP4_MPEG2TS_SYNC_BYTE;
            header[1] = ((payload_start?1:0)<<6) | (pid_ >> 8);
            header[2] = pid_ & 0xFF;

            boost::uint32_t adaptation_field_size = 0;
            if (with_pcr) adaptation_field_size += 2+AP4_MPEG2TS_PCR_ADAPTATION_SIZE;

            // clamp the payload size
            if (payload_size+adaptation_field_size > AP4_MPEG2TS_PACKET_PAYLOAD_SIZE) {
                payload_size = AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-adaptation_field_size;
            }
            // adjust the adaptation field to include stuffing if necessary
            if (adaptation_field_size+payload_size < AP4_MPEG2TS_PACKET_PAYLOAD_SIZE) {
                adaptation_field_size = AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-payload_size;
            }
            
            if (adaptation_field_size == 0) {
                // no adaptation field
                header[3] = (1<<4) | ((continuity_counter_++)&0x0F);
                ts_header_->Write(header, 4);
            } else {
                // adaptation field present
                header[3] = (3<<4) | ((continuity_counter_++)&0x0F);
                ts_header_->Write(header, 4);
                if (adaptation_field_size == 1) {
                    // just one byte (stuffing)
                    ts_header_->WriteUI08(0);
                } else {
                    // two or more bytes (stuffing and/or PCR)
                    ts_header_->WriteUI08(adaptation_field_size-1);
                    ts_header_->WriteUI08(with_pcr?(1<<4):0);
                    unsigned int pcr_size = 0;
                    if (with_pcr) {
                        pcr_size = AP4_MPEG2TS_PCR_ADAPTATION_SIZE;
                        AP4_UI64 pcr_base = pcr/300;
                        AP4_UI32 pcr_ext  = (AP4_UI32)(pcr%300);
                        AP4_BitWriter writer(pcr_size);
                        writer.Write((AP4_UI32)(pcr_base>>32), 1);
                        writer.Write((AP4_UI32)pcr_base, 32);
                        writer.Write(0x3F, 6);
                        writer.Write(pcr_ext, 9);
                        ts_header_->Write(writer.GetData(), pcr_size);
                    }
                    if (adaptation_field_size > 2) {
                        ts_header_->Write(StuffingBytes, adaptation_field_size-pcr_size-2);
                    }
                }
            }
            AP4_Position pos;
            ts_header_->Tell(pos);
            output.resize(pos);
            memcpy(&output.at(0), ts_header_->GetData(), pos);
        }

    } // namespace mux
} // namespace ppbox

