// FlvTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvTransfer.h"

using namespace ppbox::avformat;

#include <framework/system/BytesOrder.h>
using namespace framework::system;

namespace ppbox
{
    namespace mux
    {

        FlvTransfer::FlvTransfer(
            boost::uint8_t type)
            : previous_tag_size_(0)
        {
            header_.Type = type;
        }

        FlvTransfer::~FlvTransfer()
        {
        }

        void FlvTransfer::transfer(
            Sample & sample)
        {
            header_.DataSize = sample.size;
            header_.Timestamp = (boost::uint32_t)sample.time;
            header_.TimestampExtended   = (boost::uint8_t)(sample.time >> 24);
            header_.StreamID = 0x00;

            FormatBuffer buf(header_buffer_, sizeof(header_buffer_));
            FlvOArchive flv_archive(buf);
            flv_archive << header_;
            sample.data.push_front(buf.data());
            sample.size += buf.size();

            previous_tag_size_ = BytesOrder::host_to_big_endian_long(sample.size);
            sample.data.push_back(boost::asio::buffer((boost::uint8_t*)&previous_tag_size_, 4));
            sample.size += 4;

            sample.context = &header_;
        }

        void FlvTransfer::stream_header(
            StreamInfo const & info, 
            Sample & sample)
        {
        }

    } // namespace mux
} // namespace ppbox
