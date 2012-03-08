// TsStreamTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/TsTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"

#include <util/archive/ArchiveBuffer.h>

#define PES_TIME_SCALE 90000

namespace ppbox
{
    namespace mux
    {

        TsTransfer::TsTransfer(
            boost::uint16_t pid,
            boost::uint16_t stream_id)
            : Stream(pid)
            , stream_id_(stream_id)
            , time_adjust_(0)
            , with_pcr_(false)
            , with_dts_(false)
        {
        }

        TsTransfer::~TsTransfer()
        {
        }

        void TsTransfer::transfer(
            ppbox::mux::MediaInfoEx & media)
        {
            if (media.type == ppbox::demux::MEDIA_TYPE_VIDE) {
                scale_.reset(media.time_scale, PES_TIME_SCALE);
                with_pcr_ = true;
                with_dts_ = true;
            } else {
                if (media.time_scale < media.audio_format.sample_rate) {
                    scale_.reset(media.audio_format.sample_rate, PES_TIME_SCALE);
                    time_adjust_ = 1;
                } else {
                    scale_.reset(media.time_scale, PES_TIME_SCALE);
                }
            }
        }

        void TsTransfer::transfer(
            ppbox::demux::Sample & sample)
        {
            MediaInfoEx const & media = 
                *(MediaInfoEx const *)sample.media_info;
            boost::uint64_t cts = 0;
            //std::cout << "sample track = " << sample.itrack << ", dts = " << dts << ", cts = " << cts << std::endl;
            if (time_adjust_ == 0) {
                sample.dts = scale_.transfer(sample.dts);
                cts = scale_.inc(sample.cts_delta);
                sample.cts_delta = cts - sample.dts;
            } else if (time_adjust_ == 1) {
                sample.dts = cts = scale_.static_transfer(media.time_scale, PES_TIME_SCALE, sample.dts);
                scale_.set(sample.dts);
                time_adjust_ = 2;
            } else {
                sample.dts = cts = scale_.inc(1024);
            }
            //std::cout << "sample track = " << sample.itrack << ", dts = " << dts << ", cts = " << cts << std::endl;
            boost::uint32_t frame_size = sample.size;
            ts_buffers_.clear();
            ts_headers_.clear();
            MyBuffersLimit limit(sample.data.begin(), sample.data.end());
            MyBuffersPosition position(limit);
            size_t pes_header_size = 14 + (with_dts_ ? 5 : 0);
            PESPacket pes_packet;
            pes_packet.stream_id = stream_id_;
            //assert (frame_size + pes_header_size - 6 < 65536);
            pes_packet.PES_packet_length = (media.type == ppbox::demux::MEDIA_TYPE_VIDE)
                ? 0 : (frame_size + pes_header_size - 6);
            pes_packet.PES_scrambling_control = 0;
            pes_packet.PES_priority = 0;
            pes_packet.data_alignment_indicator = 1;
            pes_packet.copyright = 0;
            pes_packet.original_or_copy = 0;
            pes_packet.PTS_DTS_flags = with_dts_ ? 3 : 2;
            pes_packet.ESCR_flag = 0;
            pes_packet.ES_rate_flag = 0;
            pes_packet.DSM_trick_mode_flag = 0;
            pes_packet.additional_copy_info_flag = 0;
            pes_packet.PES_CRC_flag = 0;
            pes_packet.PES_extension_flag = 0;
            pes_packet.PES_header_data_length = pes_header_size - 9;
            pes_packet.reserved1 = with_dts_ ? 3 : 2;
            pes_packet.Pts32_30 = (cts >> 30) & 7;
            pes_packet.pts_marker_bit1 = 1;
            pes_packet.Pts29_15 = (cts >> 15) & 0x7FFF;
            pes_packet.pts_marker_bit2 = 1;
            pes_packet.Pts14_0 = cts & 0x7FFF;
            pes_packet.pts_marker_bit3 = 1;
            if (with_dts_) {
                pes_packet.reserved2 = 1;
                pes_packet.Dts32_30 = (sample.dts >> 30) & 7;
                pes_packet.dts_marker_bit1 = 1;
                pes_packet.Dts29_15 = (sample.dts >> 15) & 0x7FFF;;
                pes_packet.dts_marker_bit2 = 1;
                pes_packet.Dts14_0 = sample.dts & 0x7FFF;
                pes_packet.dts_marker_bit3 = 1;
            }

            util::archive::ArchiveBuffer<char> buf(pes_heaher_buffer_, pes_header_size);
            TsOArchive ts_archive(buf);
            ts_archive << pes_packet;

            bool first_packet = true;
            frame_size += pes_header_size; // add size of PES header

            boost::uint32_t ts_count = (frame_size + (with_pcr_ ? 8 : 0) + AP4_MPEG2TS_PACKET_PAYLOAD_SIZE - 1) / AP4_MPEG2TS_PACKET_PAYLOAD_SIZE;
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
                    WritePacketHeader(first_packet, payload_size, head_size, with_pcr_, sample.dts, ptr);
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

        void TsTransfer::on_seek(
            boost::uint32_t time)
        {
            if (time_adjust_ == 2)
                time_adjust_ = 1;
        }

    }
}
