// Mpeg2Ts.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/Mpeg2Ts.h"
using namespace ppbox::demux;
using namespace ppbox::mux;

#include <util/archive/ArchiveBuffer.h>

#include <bento4/Core/Ap4ByteStream.h>
#include <bento4/Core/Ap4Sample.h>
#include <bento4/Core/Ap4SampleDescription.h>
#include <bento4/Core/Ap4Utils.h>

#include <stdio.h>

namespace ppbox
{
    namespace mux
    {

        void Stream::WritePacketHeader(
            bool payload_start, 
            boost::uint32_t & payload_size,
            boost::uint32_t & head_size, // in: spare size, out: real size
            bool with_pcr,
            boost::uint64_t pcr,
            boost::uint8_t * ptr)
        {
            util::archive::ArchiveBuffer<char> buf((char *)ptr, head_size);
            TsOArchive ts_archive(buf);
            transport_packet_.transport_error_indicator = 0;
            if (payload_start) {
                transport_packet_.payload_uint_start_indicator = 1;
            } else {
                transport_packet_.payload_uint_start_indicator = 0;
            }
            transport_packet_.transport_priority = (pid_ == 0) ? 1 : 0;
            transport_packet_.Pid = pid_;
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
            transport_packet_.transport_scrambling_control = 0;
            if (adaptation_field_size == 0) {
                transport_packet_.adaptat_field_control = 1;
                transport_packet_.continuity_counter = (continuity_counter_++)&0x0F;
                ts_archive << transport_packet_;
                head_size = 4;
            } else {
                // adaptation field present
                transport_packet_.adaptat_field_control = 3;
                transport_packet_.continuity_counter = (continuity_counter_++)&0x0F;
                ts_archive << transport_packet_;
                if (adaptation_field_size == 1) {
                    // just one byte (stuffing)
                    adapation_field_.adaptation_field_length = 0;
                } else {
                    // two or more bytes (stuffing and/or PCR)
                    adapation_field_.adaptation_field_length = adaptation_field_size-1;
                    adapation_field_.discontinuity_indicator = 0;
                    adapation_field_.random_access_indicator = 0;
                    adapation_field_.elementary_stream_priority_indicator = 0;
                    adapation_field_.PCR_flag = 0;
                    adapation_field_.OPCR_flag = 0;
                    adapation_field_.splicing_point_flag = 0;
                    adapation_field_.transport_private_data_flag = 0;
                    adapation_field_.adaptation_field_extension_flag = 0;
                    unsigned int pcr_size = 0;
                    if (with_pcr) {
                        adapation_field_.PCR_flag = 1;
                        pcr_size = AP4_MPEG2TS_PCR_ADAPTATION_SIZE;
                        boost::uint64_t pcr_base = pcr;
                        boost::uint32_t pcr_ext  = 0;
                        adapation_field_.program_clock_reference_base = pcr_base >> 1;
                        adapation_field_.program_clock_reference_base_last1bit = pcr_base & 0x01;
                        adapation_field_.pcr_reserved = 0x3F;
                        adapation_field_.program_clock_reference_extension = pcr_ext;
                    }
                    head_size = adaptation_field_size + 4;
                    boost::int32_t suffer_size = adaptation_field_size-pcr_size-2;
                    if (adaptation_field_size > 2 && suffer_size >= 1) {
                        adapation_field_.stuffing_bytes.resize(suffer_size);
                        adapation_field_.stuffing_bytes.assign(suffer_size, 255);
                    }
                }
                ts_archive << adapation_field_;
            }
        }

    } // namespace mux
} // namespace ppbox

