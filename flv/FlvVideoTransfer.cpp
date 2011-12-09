// FlvVideoTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvVideoTransfer.h"

#include <ppbox/demux/flv/FlvFormat.h>
using namespace ppbox::demux;

using namespace framework::system;

namespace ppbox
{
    namespace mux
    {

        void FlvVideoTransfer::transfer(ppbox::demux::Sample & sample)
        {

            if(sample.flags & demux::Sample::sync) {
                videotagheader_.FrameType = 1;
            } else {
                videotagheader_.FrameType = 2;
            }

            boost::uint32_t CompositionTime = sample.cts_delta * 1000 / sample.media_info->time_scale;
            videotagheader_.CompositionTime = framework::system::UInt24(
                framework::system::BytesOrder::host_to_big_endian(CompositionTime));
            util::archive::ArchiveBuffer<char> buf(video_tag_header_, 16);
            ppbox::demux::FLVOArchive flv_archive(buf);
            flv_archive << videotagheader_;
            boost::uint32_t data_length = sample.size;
            setTagSizeAndTimestamp(data_length+5, sample.time);

            sample.data.push_front(boost::asio::buffer(video_tag_header_, 5));
            sample.data.push_front(tag_buffer());

            previous_tag_size_ = data_length + 5 + 11;
            previous_tag_size_ = BytesOrder::host_to_big_endian_long(previous_tag_size_);
            sample.data.push_back(boost::asio::buffer((boost::uint8_t*)&previous_tag_size_, 4));
            sample.size += 20;
        }

        void FlvVideoTransfer::transfer(MediaInfoEx & mediainfo)
        {
            videotagheader_.CodecID = VideoCodec::FLV_CODECID_H264;
            videotagheader_.AVCPacketType = 1;
        }

    }
}

