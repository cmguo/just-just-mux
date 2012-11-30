// FlvVideoTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/flv/FlvVideoTransfer.h"

#include <ppbox/avformat/flv/FlvFormat.h>
#include <ppbox/avformat/codec/avc/AvcCodec.h>
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
            header_.CodecID = FlvVideoCodec::H264;
            header_.AVCPacketType = 1;
        }

        void FlvVideoTransfer::transfer(
            Sample & sample)
        {
            if(sample.flags & Sample::sync) {
                header_.FrameType = 1;
            } else {
                header_.FrameType = 2;
            }
            boost::uint32_t CompositionTime = sample.cts_delta * 1000 / sample.stream_info->time_scale;
            header_.CompositionTime = CompositionTime;

            FormatBuffer buf(header_buffer_, 16);
            FlvOArchive flv_archive(buf);
            flv_archive << header_;
            sample.data.push_front(buf.data());
            sample.size += buf.size();

            FlvTransfer::transfer(sample);
        }

        void FlvVideoTransfer::stream_header(
            StreamInfo const & info, 
            Sample & sample)
        {
            if (info.sub_type == VIDEO_TYPE_AVC1) {
                header_.AVCPacketType = 0;
                AvcConfigHelper const & config = ((AvcCodec *)info.codec)->config_helper();
                config.to_data(config_data_);
                sample.data.push_back(boost::asio::buffer(config_data_));
                sample.size += config_data_.size();
                transfer(sample);
                header_.AVCPacketType = 1; // restore
            } else {
                assert(false);
            }
        }

    } // namespace mux
} // namespace ppbox
