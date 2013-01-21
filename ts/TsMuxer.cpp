// TsMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/TsMuxer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/transfer/H264PackageSplitTransfer.h"
#include "ppbox/mux/transfer/H264StreamSplitTransfer.h"
#include "ppbox/mux/transfer/H264PtsComputeTransfer.h"
#include "ppbox/mux/transfer/H264StreamJoinTransfer.h"
#include "ppbox/mux/transfer/MpegAudioAdtsEncodeTransfer.h"
#include "ppbox/mux/ts/PesTransfer.h"

using namespace ppbox::avformat;

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
            PmtSection & pmt_sec = pmt_.sections.front();
            if (info.type == MEDIA_TYPE_VIDE) {
                if (info.format_type == StreamInfo::video_avc_packet) {
                    transfer = new H264PackageSplitTransfer();
                    transfers.push_back(transfer);
                    transfer = new H264StreamJoinTransfer();
                    transfers.push_back(transfer);
                } else if (info.format_type == StreamInfo::video_avc_byte_stream) {
                    transfer = new H264StreamSplitTransfer();
                    transfers.push_back(transfer);
                    transfer = new H264PtsComputeTransfer();
                    transfers.push_back(transfer);
                }
                pmt_sec.add_stream(TsStreamType::iso_13818_video);
            } else if (info.type == MEDIA_TYPE_AUDI) {
                if (info.sub_type == AUDIO_TYPE_MP4A) {
                    pmt_sec.add_stream(TsStreamType::iso_13818_7_audio);
                    transfer = new MpegAudioAdtsEncodeTransfer();
                    transfers.push_back(transfer);
                } else if (info.sub_type == AUDIO_TYPE_MP1A) {
                    pmt_sec.add_stream(TsStreamType::iso_11172_audio);
                }
            }
            transfer = new PesTransfer(info.index);
            transfers.push_back(transfer);
        }

        void TsMuxer::file_header(
            Sample & sample)
        {
            pmt_.complete();

            FormatBuffer buf(header_buffer_, sizeof(header_buffer_));
            TsOArchive oa(buf);
            oa << pat_;
            oa << pmt_;
            assert(buf.size() == TsPacket::PACKET_SIZE * 2);

            sample.data.push_back(buf.data());
        }

        void TsMuxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
        }

    } // namespace mux
} // namespace ppbox
