// RtmTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/rtm/RtmTransfer.h"

#include <ppbox/avformat/flv/FlvTagType.h>
#include <ppbox/avformat/stream/SampleBuffers.h>
#include <ppbox/avformat/stream/FormatBuffer.h>
using namespace ppbox::avformat;

#include <util/protocol/rtmp/RtmpMessageTraits.h>

namespace ppbox
{
    namespace mux
    {

        RtmTransfer::RtmTransfer()
            : chunk_size_(128)
        {
            header_.cs_id(8);
        }

        RtmTransfer::~RtmTransfer()
        {
        }

        void RtmTransfer::transfer(
            Sample & sample)
        {
            FlvTagHeader const * tag_header = (FlvTagHeader const *)sample.context;
            util::protocol::RtmpChunkHeader chunk;
            chunk.calc_timestamp = tag_header->Timestamp; // it is really 32 bits long
            chunk.message_length = tag_header->DataSize;
            chunk.message_type_id = tag_header->Type;
            chunk.message_stream_id = 1;
            header_.dec(chunk);
            sample.data.pop_front(); // È¥³ý FlvTagHeader
            sample.data.pop_back(); // È¥³ý previous_tag_size_
            sample.size = tag_header->DataSize;

            SampleBuffers::ConstBuffers data;
            SampleBuffers::BuffersPosition beg(sample.data.begin(), sample.data.end());
            SampleBuffers::BuffersPosition end(sample.data.end());
            boost::uint32_t left = sample.size;

            FormatBuffer abuf(header_buffer_, sizeof(header_buffer_));
            util::protocol::RtmpMessageTraits::o_archive_t oa(abuf);
            oa << header_;
            data.push_back(abuf.data());
            sample.size += abuf.size();
            abuf.consume(abuf.size());

            if (left > chunk_size_) {
                header_.fmt = 3;
                oa << header_;
            }

            while (left > chunk_size_) {
                SampleBuffers::BuffersPosition pos(beg);
                beg.increment_bytes(end, chunk_size_);
                data.insert(data.end(), SampleBuffers::range_buffers_begin(pos, beg), SampleBuffers::range_buffers_end());
                data.push_back(abuf.data());
                sample.size += abuf.size();
                left -= chunk_size_;
            }

            // the last chunk or the only one chunk
            data.insert(data.end(), SampleBuffers::range_buffers_begin(beg, end), SampleBuffers::range_buffers_end());

            sample.data.swap(data);
        }

        void RtmTransfer::on_seek(
            boost::uint64_t time)
        {
            header_ = util::protocol::RtmpChunkHeader();
        }

    } // namespace mux
} // namespace ppbox
