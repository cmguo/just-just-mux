// MmsMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/mms/MmsMuxer.h"
#include "ppbox/mux/mms/MmsTransfer.h"
#include "ppbox/mux/transfer/MergeTransfer.h"

#include <util/buffers/BuffersCopy.h>

namespace ppbox
{
    namespace mux
    {

        MmsMuxer::MmsMuxer()
            : mms_transfer_(NULL)
        {
        }

        MmsMuxer::~MmsMuxer()
        {
            if (mms_transfer_) {
                delete mms_transfer_;
                mms_transfer_ = NULL;
            }
        }

        void MmsMuxer::add_stream(
            StreamInfo & info, 
            std::vector<Transfer *> & transfers)
        {
            AsfMuxer::add_stream(info, transfers);
            if (mms_transfer_ == NULL) {
                mms_transfer_ = new MmsTransfer;
            }
            Transfer * transfer = new MergeTransfer(mms_transfer_);
            transfers.push_back(transfer);
        }

        void MmsMuxer::file_header(
            Sample & sample)
        {
        }

        void MmsMuxer::media_info(
            MediaInfo & info) const
        {
            MuxerBase::media_info(info);
            Sample tag;
            const_cast<MmsMuxer *>(this)->AsfMuxer::file_header(tag);
            mms_transfer_->file_header(tag);
            info.format_data.resize(tag.size);
            util::buffers::buffers_copy(
                boost::asio::buffer(&info.format_data.at(0), info.format_data.size()), 
                tag.data);
        }

    } // namespace mux
} // namespace ppbox
