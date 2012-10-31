// FlvVideoTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvVideoTransfer.h"

#include <ppbox/avformat/flv/FlvFormat.h>
using namespace ppbox::avformat;

using namespace framework::system;

namespace ppbox
{
    namespace mux
    {

        FlvVideoTransfer::FlvVideoTransfer(
            boost::uint8_t type)
            : FlvTransfer(type)
        {
        }

        void FlvVideoTransfer::transfer(
            StreamInfo & info)
        {
            videotagheader_.CodecID = FlvVideoCodec::H264;
            videotagheader_.AVCPacketType = 1;
        }

        void FlvVideoTransfer::transfer(
            Sample & sample)
        {
            if(sample.flags & Sample::sync) {
                videotagheader_.FrameType = 1;
            } else {
                videotagheader_.FrameType = 2;
            }

            boost::uint32_t CompositionTime = sample.cts_delta * 1000 / sample.stream_info->time_scale;
            videotagheader_.CompositionTime = CompositionTime;
            util::archive::ArchiveBuffer<char> buf(video_tag_header_, 16);
            ppbox::avformat::FLVOArchive flv_archive(buf);
            flv_archive << videotagheader_;
            boost::uint32_t data_length = sample.size;
            setTagSizeAndTimestamp(data_length+5, (boost::uint32_t)sample.time);

            sample.data.push_front(boost::asio::buffer(video_tag_header_, 5));
            sample.data.push_front(tag_buffer());

            previous_tag_size_ = data_length + 5 + 11;
            previous_tag_size_ = BytesOrder::host_to_big_endian_long(previous_tag_size_);
            sample.data.push_back(boost::asio::buffer((boost::uint8_t*)&previous_tag_size_, 4));
            sample.size += 20;
        }

    } // namespace mux
} // namespace ppbox
