// TsTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/TsTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"

#include <ppbox/avformat/ts/TsPacket.h>
using namespace ppbox::avformat;

#include <util/archive/ArchiveBuffer.h>

namespace ppbox
{
    namespace mux
    {

        TsTransfer::TsTransfer(
            boost::uint16_t pid)
            : pid_(pid)
            , continuity_counter_(0)
            , with_pcr_(false)
            , time_adjust_(0)
        {
        }

        TsTransfer::~TsTransfer()
        {
        }

        void TsTransfer::transfer(
            StreamInfo & info)
        {
            if (info.type == MEDIA_TYPE_VIDE) {
                scale_.reset(info.time_scale, TsPacket::TIME_SCALE);
                with_pcr_ = true;
            } else {
                if (info.time_scale < info.audio_format.sample_rate) {
                    scale_.reset(info.audio_format.sample_rate, TsPacket::TIME_SCALE);
                    time_adjust_ = 1;
                } else {
                    scale_.reset(info.time_scale, TsPacket::TIME_SCALE);
                }
            }
        }

        void TsTransfer::transfer_time(
            Sample & sample)
        {
            StreamInfo const & media = 
                *(StreamInfo const *)sample.stream_info;
            //std::cout << "sample track = " << sample.itrack << ", dts = " << sample.dts << ", cts_delta = " << sample.cts_delta << std::endl;
            if (time_adjust_ == 0) {
                sample.dts = scale_.transfer(sample.dts);
                boost::uint64_t cts = scale_.inc(sample.cts_delta);
                sample.cts_delta = (boost::uint32_t)(cts - sample.dts);
            } else if (time_adjust_ == 1) {
                sample.dts = scale_.static_transfer(media.time_scale, TsPacket::TIME_SCALE, sample.dts);
                scale_.set(sample.dts);
                time_adjust_ = 2;
            } else {
                sample.dts = scale_.inc(1024);
            }
            //std::cout << "sample track = " << sample.itrack << ", dts = " << sample.dts << ", cts_delta = " << sample.cts_delta << std::endl;
        }

        void TsTransfer::transfer(
            Sample & sample)
        {
            boost::uint32_t frame_size = sample.size;

            MyBuffersLimit limit(sample.data.begin(), sample.data.end());
            MyBuffersPosition position(limit);

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
            util::archive::ArchiveBuffer<boost::uint8_t> buf(&header_buffer_[0], header_buffer_.size());
            TsOArchive oa(buf);

            while (frame_size) {
                boost::uint32_t payload_size = frame_size;
                MyBuffersPosition begin = position;
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
                position.increment_bytes(limit, payload_size);
                push_buffers(MyBufferIterator(limit, begin, position), MyBufferIterator());
                ++continuity_counter_;
                frame_size -= payload_size;
            }
            assert(&header_buffer_[0] + ts_head_pad_size == boost::asio::buffer_cast<boost::uint8_t const *>(buf.data()));
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

    } // namespace mux
} // namespace ppbox
