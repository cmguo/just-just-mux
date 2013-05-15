// MkvMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/mkv/MkvMuxer.h"
#include "ppbox/mux/mkv/MkvTransfer.h"

#include "ppbox/mux/transfer/MergeTransfer.h"
#include "ppbox/mux/transfer/H264PackageSplitTransfer.h"
#include "ppbox/mux/transfer/H264StreamJoinTransfer.h"

namespace ppbox
{
    namespace mux
    {

        MkvMuxer::MkvMuxer()
            : stream_number_(0)
            , transfer_(NULL)
        {
        }

        MkvMuxer::~MkvMuxer()
        {
        }

        void MkvMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            stream_number_++;

            //Transfer * transfer = NULL;
            //if (mediainfoex.type == MEDIA_TYPE_VIDE) {
                //if (mediainfoex.format_type == MediaInfo::video_avc_packet) {
                    //transfer = new H264PackageSplitTransfer();
                    //mediainfoex.transfers.push_back(transfer);
                    //transfer = new H264StreamJoinTransfer();
                    //mediainfoex.transfers.push_back(transfer);
                //}
            //}
            if (transfer_ == NULL)
                transfer_ = new MkvTransfer();
            transfers.push_back(new MergeTransfer(transfer_));

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
