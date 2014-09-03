// AsfMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/asf/AsfMuxer.h"
#include "ppbox/mux/asf/AsfTransfer.h"
#include "ppbox/mux/filter/MergeFilter.h"

#include <util/archive/BigEndianBinaryOArchive.h>

namespace ppbox
{
    namespace mux
    {

        AsfMuxer::AsfMuxer(
            boost::asio::io_service & io_svc)
            : Muxer(io_svc)
            , transfer_(NULL)
        {
        }

        AsfMuxer::~AsfMuxer()
        {
        }

        void AsfMuxer::add_stream(
            StreamInfo & info, 
            FilterPipe & pipe)
        {
             if (transfer_ == NULL)
                 transfer_ = new AsfTransfer;
             pipe.insert(new MergeFilter(transfer_));

             transfer_->stream_header(info, stream_buf_);
        }

        void AsfMuxer::file_header(
            Sample & sample)
        {
            if(0 == file_buf_.size()) {
                transfer_->file_header(media_info_, stream_buf_.size(), file_buf_);
                transfer_->data_header(data_buf_);
            }

            sample.data.push_back(file_buf_.data());
            sample.data.push_back(stream_buf_.data());
            sample.data.push_back(data_buf_.data());
        }

        void AsfMuxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
        }

    } // namespace mux
} // namespace ppbox
