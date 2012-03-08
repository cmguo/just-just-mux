// TsStreamTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/TsTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"

#include <util/archive/ArchiveBuffer.h>

namespace ppbox
{
    namespace mux
    {

        void TsTransfer::transfer(ppbox::demux::Sample & sample)
        {
            MediaInfoEx const * stream_info = 
                (MediaInfoEx const *)sample.media_info;
            boost::uint64_t dts = sample.dts;
            boost::uint64_t cts = sample.dts + (boost::int32_t)sample.cts_delta;
            //dts = AP4_ConvertTime(dts, timescale_, 90000);
            //cts = AP4_ConvertTime(cts, timescale_, 90000);
            dts = dts * 90000 / timescale_;
            cts = cts * 90000 / timescale_;
            if (stream_info->type == ppbox::demux::MEDIA_TYPE_VIDE) {
                WritePES(sample, dts, true, cts, true);
            } else {
                WritePES(sample, dts, false, cts, false);
            }
        }

        void TsTransfer::WritePES(
            ppbox::demux::Sample & sample,
            boost::uint64_t dts, 
            bool with_dts, 
            boost::uint64_t pts, 
            bool with_pcr)
        {
            MediaInfoEx const * stream_info = 
                (MediaInfoEx const *)sample.media_info;
            boost::uint32_t frame_size = sample.size;
            ts_buffers_.clear();
            ts_headers_.clear();
            if (frame_size == 0) {
                return;
            }
            //dts += 117000;
            //pts += 117000;
            MyBuffersLimit limit(sample.data.begin(), sample.data.end());
            MyBuffersPosition position(limit);
            size_t pes_header_size = 14 + (with_dts ? 5 : 0);
            PESPacket pes_packet;
            pes_packet.stream_id = stream_id_;
            assert (frame_size + pes_header_size - 6 < 655356);
            pes_packet.PES_packet_length = (stream_info->type == ppbox::demux::MEDIA_TYPE_VIDE)
                ? 0 : (frame_size + pes_header_size - 6);
            pes_packet.PES_scrambling_control = 0;
            pes_packet.PES_priority = 0;
            pes_packet.data_alignment_indicator = 1;
            pes_packet.copyright = 0;
            pes_packet.original_or_copy = 0;
            pes_packet.PTS_DTS_flags = with_dts ? 3 : 2;
            pes_packet.ESCR_flag = 0;
            pes_packet.ES_rate_flag = 0;
            pes_packet.DSM_trick_mode_flag = 0;
            pes_packet.additional_copy_info_flag = 0;
            pes_packet.PES_CRC_flag = 0;
            pes_packet.PES_extension_flag = 0;
            pes_packet.PES_header_data_length = pes_header_size - 9;
            pes_packet.reserved1 = with_dts ? 3 : 2;
            pes_packet.Pts32_30 = (pts >> 30) & 7;
            pes_packet.pts_marker_bit1 = 1;
            pes_packet.Pts29_15 = (pts>>15) & 0x7FFF;
            pes_packet.pts_marker_bit2 = 1;
            pes_packet.Pts14_0 = pts & 0x7FFF;
            pes_packet.pts_marker_bit3 = 1;
            if (with_dts) {
                pes_packet.reserved2 = 1;
                pes_packet.Dts32_30 = (dts >> 30) & 7;
                pes_packet.dts_marker_bit1 = 1;
                pes_packet.Dts29_15 = (dts>>15) & 0x7FFF;;
                pes_packet.dts_marker_bit2 = 1;
                pes_packet.Dts14_0 = dts & 0x7FFF;
                pes_packet.dts_marker_bit3 = 1;
            }

            //dts -= 117000;
            //pts -= 117000;
            util::archive::ArchiveBuffer<char> buf(pes_heaher_buffer_, pes_header_size);
            TsOArchive ts_archive(buf);
            ts_archive << pes_packet;

            bool first_packet = true;
            frame_size += pes_header_size; // add size of PES header

            boost::uint32_t ts_count = (frame_size + (with_pcr ? 8 : 0) + AP4_MPEG2TS_PACKET_PAYLOAD_SIZE - 1) / AP4_MPEG2TS_PACKET_PAYLOAD_SIZE;
            boost::uint32_t ts_total_size = ts_count * (AP4_MPEG2TS_PACKET_PAYLOAD_SIZE + 4);
            boost::uint32_t ts_head_pad_size = ts_total_size - frame_size;
            if (ts_head_pad_size > ts_headers_.size())
                ts_headers_.resize(ts_head_pad_size);
            boost::uint8_t * ptr = &ts_headers_.at(0);

            off_segs_.clear();
            while (frame_size) {
                boost::uint32_t head_size = 0;
                boost::uint32_t payload_size = frame_size;
                MyBuffersPosition begin = position;
                off_segs_.push_back(ts_buffers_.size());
                head_size = ts_head_pad_size;
                if (first_packet)  {
                    WritePacketHeader(first_packet, payload_size, head_size, with_pcr, (with_dts?dts:pts)*300, ptr);
                    ts_buffers_.push_back(boost::asio::buffer(ptr, head_size));
                    ts_head_pad_size -= head_size;
                    ptr += head_size;
                    push_buffers(boost::asio::buffer(pes_heaher_buffer_, pes_header_size));
                    position.increment_bytes(limit, payload_size - pes_header_size);
                    push_buffers(MyBufferIterator(limit, begin, position), MyBufferIterator());
                    first_packet = false;
                } else {
                    WritePacketHeader(first_packet, payload_size, head_size, false, 0, ptr);
                    ts_buffers_.push_back(boost::asio::buffer(ptr, head_size));
                    ts_head_pad_size -= head_size;
                    ptr += head_size;
                    position.increment_bytes(limit, payload_size);
                    push_buffers(MyBufferIterator(limit, begin, position), MyBufferIterator());
                }
                frame_size -= payload_size;
            }
            assert(ts_head_pad_size == 0);
            sample.data.assign(ts_buffers_.begin(), ts_buffers_.end());
            sample.size = ts_total_size;
            sample.context = &off_segs_;
        }

    }
}
