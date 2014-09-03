// MkvMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/mkv/MkvMuxer.h"
#include "ppbox/mux/mkv/MkvTransfer.h"
#include "ppbox/mux/filter/MergeFilter.h"

namespace ppbox
{
    namespace mux
    {

        MkvMuxer::MkvMuxer(
            boost::asio::io_service & io_svc)
            : Muxer(io_svc)
            , stream_number_(0)
            , transfer_(NULL)
        {
        }

        MkvMuxer::~MkvMuxer()
        {
        }

        void MkvMuxer::add_stream(
            StreamInfo & info, 
            FilterPipe & pipe)
        {
            stream_number_++;

            if (transfer_ == NULL)
                transfer_ = new MkvTransfer();
            pipe.insert(new MergeFilter(transfer_));

            transfer_->stream_header(info, track_buf_);
        }

        void MkvMuxer::file_header(
            ppbox::demux::Sample & sample)
        {
            if(header_buf_.size() == 0) {
                transfer_->file_header(media_info_, track_buf_.size(), header_buf_);
            }

            sample.data.push_back(header_buf_.data());
            sample.data.push_back(track_buf_.data());
        }

        void MkvMuxer::stream_header(
            boost::uint32_t index, 
            ppbox::demux::Sample & sample)
        {
        }

    } // namespace mux
} // namespace ppbox
