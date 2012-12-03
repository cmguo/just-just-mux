// PesTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/PesTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"

#include <ppbox/avformat/ts/PesPacket.h>
using namespace ppbox::avformat;

#include <util/archive/ArchiveBuffer.h>

#define PES_TIME_SCALE 90000

namespace ppbox
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
            if (info.type == MEDIA_TYPE_VIDE) {
                stream_id_ = TsStreamId::video_base + (boost::uint16_t)info.index;
                with_dts_ = true;
            } else {
                stream_id_ = TsStreamId::audio_base + (boost::uint16_t)info.index;
            }
        }

        void PesTransfer::transfer(
            Sample & sample)
        {
            TimeScaleTransfer::transfer(sample);

            FormatBuffer buf(pes_heaher_buffer_, sizeof(pes_heaher_buffer_));
            TsOArchive oa(buf);
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
} // namespace ppbox
