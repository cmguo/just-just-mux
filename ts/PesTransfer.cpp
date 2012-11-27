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
            boost::uint32_t index, 
            bool video)
            : TsTransfer(TsPid::stream_base + (boost::uint16_t)index)
            , stream_id_((video ? TsStreamId::video_base : TsStreamId::audio_base) + (boost::uint16_t)index)
            , with_dts_(video)
        {
        }

        PesTransfer::~PesTransfer()
        {
        }

        void PesTransfer::transfer(
            Sample & sample)
        {
            TsTransfer::transfer_time(sample);

            util::archive::ArchiveBuffer<boost::uint8_t> buf(pes_heaher_buffer_, sizeof(pes_heaher_buffer_));
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
