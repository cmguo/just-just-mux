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

#include <ppbox/avformat/ts/TsFormat.h>
using namespace ppbox::avformat;

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
            TsFormat ts;
            CodecInfo const * codec = ts.codec_from_codec(info.type, info.sub_type);
            if (codec) {
                pmt_sec.add_stream((boost::uint8_t)codec->format);
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
