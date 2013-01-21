// AsfMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/asf/AsfMuxer.h"
#include "ppbox/mux/asf/AsfTransfer.h"
#include "ppbox/mux/transfer/MergeTransfer.h"
#include "ppbox/mux/transfer/H264PackageSplitTransfer.h"
#include "ppbox/mux/transfer/H264StreamJoinTransfer.h"

using namespace ppbox::avformat;

#include <util/archive/BigEndianBinaryOArchive.h>

namespace ppbox
{
    namespace mux
    {

        AsfMuxer::AsfMuxer()
            : transfer_(NULL)
        {
        }

        AsfMuxer::~AsfMuxer()
        {
        }

        void AsfMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
             Transfer * transfer = NULL;
             if (info.type == MEDIA_TYPE_VIDE) {
                 if (info.format_type == StreamInfo::video_avc_packet) {
                     transfer = new H264PackageSplitTransfer();
                     transfers.push_back(transfer);
                     transfer = new H264StreamJoinTransfer();
                     transfers.push_back(transfer);
                 }
             }
             if (transfer_ == NULL)
                 transfer_ = new AsfTransfer;
             transfer=  new MergeTransfer(transfer_);
             transfers.push_back(transfer);

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
