// RtmMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/rtm/RtmMuxer.h"
#include "ppbox/mux/rtm/RtmTransfer.h"
#include "ppbox/mux/transfer/MergeTransfer.h"

#include <ppbox/avformat/flv/FlvDataType.h>
#include <ppbox/avformat/stream/FormatBuffer.h>
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        RtmMuxer::RtmMuxer()
            : rtm_transfer_(NULL)
        {
        }

        RtmMuxer::~RtmMuxer()
        {
            if (rtm_transfer_) {
                delete rtm_transfer_;
                rtm_transfer_ = NULL;
            }
        }

        void RtmMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            FlvMuxer::add_stream(info, transfers);
            if (rtm_transfer_ == NULL) {
                rtm_transfer_ = new RtmTransfer;
            }
            Transfer * transfer = new MergeTransfer(rtm_transfer_);
            transfers.push_back(transfer);
        }

        void RtmMuxer::file_header(
            Sample & sample)
        {
            //FlvDataValue v1("|RtmpSampleAccess");
            //FlvDataValue v2(FlvDataType::BOOL);
            //v2.Bool = 0;
            //FlvDataValue v3(FlvDataType::BOOL);
            //v3.Bool = 0;

            //FormatBuffer buf(header_buffer_, sizeof(header_buffer_));
            //FlvOArchive oa(buf);
            //oa << v1 << v2 << v3;

            //sample.data.push_back(buf.data());
            //sample.size = buf.size();
            //buf.consume(buf.size());
            //rtm_transfer_->file_header(sample);
            //boost::uint32_t size = boost::asio::buffer_size(sample.data.front());
            //util::buffers::buffers_copy(buf.prepare(size);

            FlvMuxer::file_header(sample);
            sample.data.pop_front(); // delete FlvHeader
            sample.size -= 9 + 4;
            rtm_transfer_->transfer(sample);
        }

        void RtmMuxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
            FlvMuxer::stream_header(index, sample);
            rtm_transfer_->transfer(sample);
        }

    } // namespace mux
} // namespace ppbox
