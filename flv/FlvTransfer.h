#ifndef   _PPBOX_MUX_FLV_TRANSFER_
#define   _PPBOX_MUX_FLV_TRANSFER_

#include "ppbox/mux/MuxBase.h"
#include "ppbox/mux/transfer/Transfer.h"

#include <ppbox/avformat/flv/FlvFormat.h>
#include <ppbox/avformat/flv/FlvTagType.h>

#include <util/archive/ArchiveBuffer.h>

#include <framework/system/BytesOrder.h>

namespace ppbox
{
    namespace mux
    {
        class FlvTransfer
            : public Transfer
        {
        public:
            FlvTransfer(boost::uint8_t type)
                : previous_tag_size_(0)
            {
                flvtag_.Type = type;
            }

            virtual ~FlvTransfer()
            {
            }

            void setTagSizeAndTimestamp(
                boost::uint32_t size, 
                boost::uint32_t timestamp)
            {
                flvtag_.DataSize = framework::system::UInt24(size);
                flvtag_.Timestamp = framework::system::UInt24(timestamp);
                flvtag_.TimestampExtended   = 0x00;
                flvtag_.StreamID = 0x00;
            }

            boost::asio::const_buffer tag_buffer()
            {
                util::archive::ArchiveBuffer<char> buf(tag_header_, 16);
                ppbox::avformat::FLVOArchive flv_archive(buf);
                flv_archive << flvtag_;
                return boost::asio::buffer(boost::asio::buffer(tag_header_, 11));
            }

        protected:
            boost::uint32_t previous_tag_size_;

        private:
            ppbox::avformat::FlvTagHeader   flvtag_;
            char tag_header_[16];

        };
    }
}

#endif // End _PPBOX_MUX_FLV_TRANSFER_
