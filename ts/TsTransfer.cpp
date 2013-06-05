// TsTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/TsTransfer.h"

#include <ppbox/avformat/ts/TsPacket.h>
#include <ppbox/avformat/ts/TsArchive.h>
using namespace ppbox::avformat;

#include <ppbox/avbase/stream/SampleBuffers.h>
using namespace ppbox::avbase;

#include <util/archive/ArchiveBuffer.h>

namespace ppbox
{
    namespace mux
    {

        TsTransfer::TsTransfer(
            boost::uint16_t pid, 
            bool with_pcr)
            : pid_(pid)
            , continuity_counter_(0)
            , with_pcr_(with_pcr)
        {
        }

        TsTransfer::~TsTransfer()
        {
        }

        void TsTransfer::transfer(
            Sample & sample)
        {
            boost::uint32_t frame_size = sample.size;

            SampleBuffers::BuffersPosition position(sample.data.begin(), sample.data.end());
            SampleBuffers::BuffersPosition end(sample.data.end());

            ts_buffers_.clear();
            header_buffer_.clear();
            off_segs_.clear();

            bool first_packet = true;

            boost::uint32_t payload_size = frame_size + (with_pcr_ ? 8 : 0);
            boost::uint32_t ts_count = (payload_size + TsPacket::PAYLOAD_SIZE - 1) / TsPacket::PAYLOAD_SIZE;
            boost::uint32_t ts_total_size = ts_count * (TsPacket::PACKET_SIZE);
            boost::uint32_t ts_head_pad_size = ts_total_size - frame_size;

            if (ts_head_pad_size > header_buffer_.size())
                header_buffer_.resize(ts_head_pad_size);
            FormatBuffer buf(&header_buffer_[0], header_buffer_.size());
            TsOArchive oa(buf);

            while (frame_size) {
                boost::uint32_t payload_size = frame_size;
                SampleBuffers::BuffersPosition begin = position;
                off_segs_.push_back(ts_buffers_.size());
                if (first_packet)  {
                    if (with_pcr_) {
                        TsPacket packet(pid_, continuity_counter_, sample.dts);
                        packet.fill_payload(payload_size);
                        oa << packet;
                    } else {
                        TsPacket packet(true, pid_, continuity_counter_);
                        packet.fill_payload(payload_size);
                        oa << packet;
                    }
                    first_packet = false;
                } else {
                    TsPacket packet(false, pid_, continuity_counter_);
                    packet.fill_payload(payload_size);
                    oa << packet;
                }
                ts_buffers_.push_back(buf.data());
                buf.consume(buf.size());
                position.increment_bytes(end, payload_size);
                push_buffers(SampleBuffers::range_buffers_begin(begin, position), SampleBuffers::range_buffers_end());
                ++continuity_counter_;
                frame_size -= payload_size;
            }
            assert(&header_buffer_[0] + ts_head_pad_size == boost::asio::buffer_cast<boost::uint8_t const *>(buf.data()));
            sample.data.assign(ts_buffers_.begin(), ts_buffers_.end());
            sample.size = ts_total_size;
            sample.context = &off_segs_;
        }

    } // namespace mux
} // namespace ppbox
