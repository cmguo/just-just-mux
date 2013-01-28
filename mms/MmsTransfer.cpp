// MmsTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/asf/AsfTransfer.h"
#include "ppbox/mux/mms/MmsTransfer.h"

using namespace ppbox::avformat;

#include <framework/string/Format.h>
#include <framework/string/Base64.h>

#include <util/buffers/BuffersSize.h>
#include <util/protocol/mmsp/MmspFormat.h>
#include <util/protocol/mmsp/MmspViewerToMacMessage.h>

namespace ppbox
{
    namespace mux
    {

        MmsTransfer::MmsTransfer()
        {
            header_.playIncarnation = 4;
            header_.PacketSize = header_.HEAD_SIZE + 1024;
        }

        MmsTransfer::~MmsTransfer()
        {
        }

        void MmsTransfer::transfer(
            Sample & sample)
        {
            // Don't need adjust time scale, asf transfer already done it
            //RtpTransfer::transfer(sample);

            std::vector<AsfTransfer::AsfPacket> const & packets = 
                *(std::vector<AsfTransfer::AsfPacket> const *)sample.context;
            // packets里面多记录了一个不完整packet
            size_t buffer_size = (packets.size() - 1) * header_.HEAD_SIZE;
            if (buffer_size > header_buffer_.size()) {
                header_buffer_.resize(buffer_size);
            }

            std::deque<boost::asio::const_buffer> data;
            util::archive::ArchiveBuffer<boost::uint8_t> buf(&header_buffer_.front(), header_buffer_.size());
            util::protocol::MsspOArchive oa(buf);
            std::deque<boost::asio::const_buffer>::const_iterator buf_beg = sample.data.begin();
            std::deque<boost::asio::const_buffer>::const_iterator buf_end = sample.data.end();
            for (size_t i = 0; i + 1 < packets.size(); ++i) {
                AsfTransfer::AsfPacket const & packet = packets[i];
                oa << header_;
                data.push_back(buf.data());
                buf_end = sample.data.begin() + packets[i + 1].off_seg;
                data.insert(data.end(), buf_beg, buf_end);

                buf.consume(header_.HEAD_SIZE);
                ++header_.LocationId;
                ++header_.AFFlags;
                buf_beg = buf_end;
            }

            sample.data.swap(data);
            sample.size += buffer_size;
        }

        void MmsTransfer::file_header(
            Sample & sample)
        {
            sample.size = util::buffers::buffers_size(sample.data);
            sample.size += header_.HEAD_SIZE;

            util::protocol::MmspDataHeader header;
            header.playIncarnation = 2;
            header.AFFlags = 0x0C;
            header.PacketSize = (boost::uint16_t)sample.size;
            if (header_buffer_.size() < header.HEAD_SIZE) {
                header_buffer_.resize(header.HEAD_SIZE);
            }
            util::archive::ArchiveBuffer<boost::uint8_t> buf(&header_buffer_.front(), header_buffer_.size());
            util::protocol::MsspOArchive oa(buf);
            oa << header;
            sample.data.push_front(buf.data());
        }

    } // namespace mux
} // namespace ppbox
