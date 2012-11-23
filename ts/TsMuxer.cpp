// TsMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/TsMuxer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/transfer/PackageSplitTransfer.h"
#include "ppbox/mux/transfer/StreamSplitTransfer.h"
#include "ppbox/mux/transfer/PtsComputeTransfer.h"
#include "ppbox/mux/transfer/StreamJoinTransfer.h"
#include "ppbox/mux/transfer/AdtsAudioTransfer.h"
#include "ppbox/mux/transfer/AudioMergeTransfer.h"
#include "ppbox/mux/ts/PesTransfer.h"

using namespace ppbox::avformat;

#include <util/archive/ArchiveBuffer.h>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        TsMuxer::TsMuxer()
        {
        }

        TsMuxer::~TsMuxer()
        {
        }

        void TsMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            Transfer * transfer = NULL;
            if (info.type == MEDIA_TYPE_VIDE) {
                if (info.format_type == StreamInfo::video_avc_packet) {
                    transfer = new PackageSplitTransfer();
                    transfers.push_back(transfer);
                    transfer = new StreamJoinTransfer();
                    transfers.push_back(transfer);
                } else if (info.format_type == StreamInfo::video_avc_byte_stream) {
                    transfer = new StreamSplitTransfer();
                    transfers.push_back(transfer);
                    transfer = new PtsComputeTransfer();
                    transfers.push_back(transfer);
                }
                pmt_.add_stream(true, TsStreamType::iso_13818_video, info.index);
                transfer = new PesTransfer(info.index, true);
                transfers.push_back(transfer);
            } else if (info.type == MEDIA_TYPE_AUDI) {
                if (info.sub_type == AUDIO_TYPE_MP4A) {
                    pmt_.add_stream(false, TsStreamType::iso_13818_7_audio, info.index);
                    transfer = new AdtsAudioTransfer();
                    transfers.push_back(transfer);
                } else if (info.sub_type == AUDIO_TYPE_MP1A) {
                    pmt_.add_stream(false, TsStreamType::iso_11172_audio, info.index);
                }
                transfer = new PesTransfer(info.index, false);
                transfers.push_back(transfer);
            }
        }

        void TsMuxer::file_header(
            Sample & sample)
        {
            pmt_.complete();
            util::archive::ArchiveBuffer<boost::uint8_t> buf(header_buffer_, sizeof(header_buffer_));
            TsOArchive oa(buf);

            oa << pat_;
            assert(buf.size() == TsPacket::PACKET_SIZE - 4); // minus crc
            buf.consume(pat_.header_adaptation_size() + 1); // skip ts header, adaptation and the pointer field
            boost::uint32_t crc = psi_calc_crc(buf.data());
            oa << crc;
            buf.consume(buf.size());

            oa << pmt_;
            assert(buf.size() == TsPacket::PACKET_SIZE - 4); // minus crc
            buf.consume(pmt_.header_adaptation_size() + 1); // skip ts header, adaptation and the pointer field
            crc = psi_calc_crc(buf.data());
            oa << crc;
            buf.consume(buf.size());

            sample.data.clear();
            sample.data.push_back(boost::asio::buffer(header_buffer_, 376));
        }

        void TsMuxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
            sample.size = 0;
            sample.data.clear();
        }

    } // namespace mux
} // namespace ppbox
