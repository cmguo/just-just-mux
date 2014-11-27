// PesTransfer.cpp

#include "just/mux/Common.h"
#include "just/mux/mp2/PesTransfer.h"

#include <just/avformat/mp2/PesPacket.h>
#include <just/avformat/mp2/Mp2Enum.h>
#include <just/avformat/mp2/Mp2Archive.h>
#include <just/avformat/Format.h>
using namespace just::avformat;

#define PES_TIME_SCALE 90000

namespace just
{
    namespace mux
    {

        PesTransfer::PesTransfer(
            boost::uint32_t index)
            : TsTransfer(TsPid::stream_base + (boost::uint16_t)index, index == 0)
            , stream_id_(0)
            , with_dts_(false)
        {
        }

        PesTransfer::~PesTransfer()
        {
        }

        void PesTransfer::transfer(
            StreamInfo & info)
        {
            TsTransfer::transfer(info);

            if (info.type == StreamType::VIDE) {
                stream_id_ = Mp2StreamId::video_base + (boost::uint16_t)info.index;
                with_dts_ = true;
            } else {
                stream_id_ = Mp2StreamId::audio_base + (boost::uint16_t)info.index;
            }
        }

        void PesTransfer::transfer(
            Sample & sample)
        {
            FormatBuffer buf(pes_heaher_buffer_, sizeof(pes_heaher_buffer_));
            Mp2OArchive oa(buf);
            if (with_dts_) {
                PesPacket pes_packet(stream_id_, sample.size, sample.dts + sample.cts_delta, sample.dts);
                oa << pes_packet;
            } else {
                PesPacket pes_packet(stream_id_, sample.size, sample.dts + sample.cts_delta);
                oa << pes_packet;
            }
            sample.size += buf.size();
            sample.data.push_front(buf.data());

            TsTransfer::transfer(sample);
        }

    } // namespace mux
} // namespace just
