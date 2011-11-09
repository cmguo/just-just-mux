// TsStreamTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/TsTransfer.h"

namespace ppbox
{
    namespace mux
    {

        void TsTransfer::transfer(ppbox::demux::Sample & sample)
        {
            ppbox::demux::MediaInfo const * stream_info = 
                (ppbox::demux::MediaInfo const *)sample.media_info;
            boost::uint64_t dts = sample.dts;
            boost::uint64_t cts = sample.dts + (boost::int32_t)sample.cts_delta;
            dts = AP4_ConvertTime(dts, timescale_, 90000);
            cts = AP4_ConvertTime(cts, timescale_, 90000);
            if (stream_info->type == ppbox::demux::MEDIA_TYPE_VIDE) {
                WritePES(sample, dts, true, cts, true);
            } else {
                WritePES(sample, 0, false, dts, false);
            }
            sample.data.assign(ts_buffers_.begin(), ts_buffers_.end());
            sample.size = util::buffers::buffer_size(ts_buffers_);
            sample.context = (void *)&off_segs_;
        }

        void TsTransfer::WritePES(
            ppbox::demux::Sample & sample,
            boost::uint64_t dts, 
            bool with_dts, 
            boost::uint64_t pts, 
            bool with_pcr)
        {
            boost::uint32_t frame_size = util::buffers::buffer_size(sample.data);
            ts_buffers_.clear();
            ts_headers_.clear();
            if (frame_size == 0) {
                return;
            }
            MyBuffersLimit limit(sample.data.begin(), sample.data.end());
            MyBuffersPosition position(limit);

            boost::uint32_t pes_header_size = 14+(with_dts?5:0);
            AP4_BitWriter pes_header(pes_header_size);
            pes_header.Write(0x000001, 24);    // packet_start_code_prefix
            pes_header.Write(stream_id_, 8);   // stream_id
            pes_header.Write(stream_id_ == AP4_MPEG2_TS_DEFAULT_STREAM_ID_VIDEO?0:(frame_size+pes_header_size-6), 16); // PES_packet_length
            pes_header.Write(2, 2);            // '01'
            pes_header.Write(0, 2);            // PES_scrambling_control
            pes_header.Write(0, 1);            // PES_priority
            pes_header.Write(1, 1);            // data_alignment_indicator
            pes_header.Write(0, 1);            // copyright
            pes_header.Write(0, 1);            // original_or_copy
            pes_header.Write(with_dts?3:2, 2); // PTS_DTS_flags
            pes_header.Write(0, 1);            // ESCR_flag
            pes_header.Write(0, 1);            // ES_rate_flag
            pes_header.Write(0, 1);            // DSM_trick_mode_flag
            pes_header.Write(0, 1);            // additional_copy_info_flag
            pes_header.Write(0, 1);            // PES_CRC_flag
            pes_header.Write(0, 1);            // PES_extension_flag
            pes_header.Write(pes_header_size-9, 8);// PES_header_data_length
            pes_header.Write(with_dts?3:2, 4);         // '0010' or '0011'
            pes_header.Write((AP4_UI32)(pts>>30), 3);  // PTS[32..30]
            pes_header.Write(1, 1);                    // marker_bit
            pes_header.Write((AP4_UI32)(pts>>15), 15); // PTS[29..15]
            pes_header.Write(1, 1);                    // marker_bit
            pes_header.Write((AP4_UI32)pts, 15);       // PTS[14..0]
            pes_header.Write(1, 1);                    // market_bit
            if (with_dts) {
                pes_header.Write(1, 4);                    // '0001'
                pes_header.Write((AP4_UI32)(dts>>30), 3);  // DTS[32..30]
                pes_header.Write(1, 1);                    // marker_bit
                pes_header.Write((AP4_UI32)(dts>>15), 15); // DTS[29..15]
                pes_header.Write(1, 1);                    // marker_bit
                pes_header.Write((AP4_UI32)dts, 15);       // DTS[14..0]
                pes_header.Write(1, 1);                    // market_bit
            }

            pes_heaher_buffer_.resize(pes_header_size);
            memcpy(&pes_heaher_buffer_.at(0), pes_header.GetData(), pes_header_size);

            bool first_packet = true;
            frame_size += pes_header_size; // add size of PES header

            while (frame_size) {
                boost::uint32_t payload_size = frame_size;
                if (payload_size > AP4_MPEG2TS_PACKET_PAYLOAD_SIZE) {
                    payload_size = AP4_MPEG2TS_PACKET_PAYLOAD_SIZE;
                }

                std::vector<boost::uint8_t> ts_header;
                off_segs_.push_back(0);
                MyBuffersPosition begin = position;
                if (first_packet)  {
                    WritePacketHeader(first_packet, payload_size, with_pcr, (with_dts?dts:pts)*300, ts_header);
                    ts_headers_.push_back(ts_header);
                    ts_buffers_.push_back(
                        boost::asio::buffer(ts_headers_[ts_headers_.size()-1]));
                    push_buffers(boost::asio::buffer(pes_heaher_buffer_));
                    //payload_size -= pes_header_size;
                    position.increment_bytes(limit, payload_size-pes_header_size);
                    push_buffers(MyBufferIterator(limit, begin, position), MyBufferIterator());
                    first_packet = false;
                    off_segs_.push_back(ts_buffers_.size());
                } else {
                    WritePacketHeader(first_packet, payload_size, false, 0, ts_header);
                    ts_headers_.push_back(ts_header);
                    ts_buffers_.push_back(boost::asio::buffer(
                        &ts_headers_[ts_headers_.size()-1].at(0), ts_headers_[ts_headers_.size()-1].size()));
                    position.increment_bytes(limit, payload_size);
                    push_buffers(MyBufferIterator(limit, begin, position), MyBufferIterator());
                    off_segs_.push_back(ts_buffers_.size());
                }
                off_segs_.pop_back();
                frame_size -= payload_size;
            }
        }

    }
}
