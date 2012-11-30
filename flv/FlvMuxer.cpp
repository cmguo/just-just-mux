// FlvMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvMuxer.h"
#include "ppbox/mux/transfer/PackageJoinTransfer.h"
#include "ppbox/mux/transfer/StreamSplitTransfer.h"
#include "ppbox/mux/transfer/PtsComputeTransfer.h"
#include "ppbox/mux/flv/FlvAudioTransfer.h"
#include "ppbox/mux/flv/FlvVideoTransfer.h"

using namespace ppbox::avformat;

#include <framework/system/BytesOrder.h>
using namespace framework::system;
using namespace boost::system;

namespace ppbox
{
    namespace mux
    {
        FlvMuxer::FlvMuxer()
        {
        }

        FlvMuxer::~FlvMuxer()
        {
        }

        void FlvMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            Transfer * transfer = NULL;
            if (info.type == MEDIA_TYPE_VIDE) {
                if (info.format_type == StreamInfo::video_avc_packet) {
                    // empty
                } else if (info.format_type == StreamInfo::video_avc_byte_stream) {
                    transfer = new StreamSplitTransfer();
                    transfers.push_back(transfer);
                    transfer = new PtsComputeTransfer();
                    transfers.push_back(transfer);
                    transfer = new PackageJoinTransfer();
                    transfers.push_back(transfer);
                }
                transfer = new FlvVideoTransfer(9);
                transfers.push_back(transfer);
            } else if (info.type == MEDIA_TYPE_AUDI) {
                transfer = new FlvAudioTransfer(8);
                transfers.push_back(transfer);
            }
        }

        void FlvMuxer::file_header(
            Sample & sample)
        {
            FlvHeader flv_header_;

            FormatBuffer buf(header_buffer_, sizeof(header_buffer_));
            ppbox::avformat::FlvOArchive archive(buf);
            archive << flv_header_;
            sample.data.push_back(buf.data());
            sample.size = buf.size();
        }

        void FlvMuxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
            StreamInfo const & stream_info = streams_[index];
            transfers_[index]->stream_header(stream_info, sample);
        }

    } // namespace mux
} // namespace ppbox
