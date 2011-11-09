// FlvVideoTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvVideoTransfer.h"

#include <framework/system/BytesOrder.h>
#include <util/buffers/BufferSize.h>
using namespace framework::system;

namespace ppbox
{
    namespace mux
    {

        void FlvVideoTransfer::transfer(ppbox::demux::Sample & sample)
        {
            flvtag_.TagType = TAG_TYPE_VIDEO;
            VideoTagHeader videotagheader;
            if(sample.flags & demux::Sample::sync) {
                videotagheader.VideoAttribute = 0x17;
            } else {
                videotagheader.VideoAttribute = 0x27;
            }
            videotagheader.AVCPacketType = 0x01;
            memset(videotagheader.CompositionTime, 0, sizeof(videotagheader.CompositionTime));
            boost::uint32_t data_length = sample.size;
            setTagSizeAndTimestamp(flvtag_, data_length+5, sample.time);
            sample_head_buffer_.resize(16);
            memcpy(&sample_head_buffer_.at(0), (boost::uint8_t*)&flvtag_, 11);
            memcpy(&sample_head_buffer_.at(0)+11, (boost::uint8_t*)&videotagheader, 5);
            sample.data.push_front(boost::asio::buffer(&sample_head_buffer_.at(0), sample_head_buffer_.size()));
            previous_tag_size_ = data_length + sizeof(VideoTagHeader) + 11;
            previous_tag_size_ = BytesOrder::host_to_big_endian_long(previous_tag_size_);
            sample.data.push_back(boost::asio::buffer((boost::uint8_t*)&previous_tag_size_, 4));
            sample.size += 20;
        }

    }
}

