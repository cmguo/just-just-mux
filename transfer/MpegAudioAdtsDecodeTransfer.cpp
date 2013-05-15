// MpegAudioAdtsDecodeTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/MpegAudioAdtsDecodeTransfer.h"

#include <ppbox/avcodec/aac/AacAdts.h>
using namespace ppbox::avcodec;

#include <ppbox/avbase/stream/BitsIStream.h>
#include <ppbox/avbase/stream/SampleBuffers.h>
using namespace ppbox::avbase;

#include <util/buffers/CycleBuffers.h>

namespace ppbox
{
    namespace mux
    {

        MpegAudioAdtsDecodeTransfer::MpegAudioAdtsDecodeTransfer()
        {
        }

        MpegAudioAdtsDecodeTransfer::~MpegAudioAdtsDecodeTransfer()
        {
        }

        void MpegAudioAdtsDecodeTransfer::transfer(
            StreamInfo & info)
        {
        }

        void MpegAudioAdtsDecodeTransfer::transfer(
            Sample & sample)
        {
            util::buffers::CycleBuffers<std::deque<boost::asio::const_buffer>, boost::uint8_t> buf(sample.data);
            buf.commit(sample.size);
            BitsIStream<boost::uint8_t> bits_is(buf);

            SampleBuffers::BuffersPosition position(sample.data.begin(), sample.data.end());
            SampleBuffers::BuffersPosition end(sample.data.end());

            std::deque<boost::asio::const_buffer> data;

            while (buf.in_avail()) {
                AacAdts adts;
                bits_is >> adts;
                size_t adts_size = buf.in_position() - position.skipped_bytes();
                sample.size -= adts_size;
                SampleBuffers::BuffersPosition position2(position);
                position2.increment_bytes(end, adts_size);;
                position.increment_bytes(end, adts.frame_length);
                bits_is.seekg((size_t)adts.frame_length - adts_size, std::ios::cur);
                data.insert(data.end(), SampleBuffers::range_buffers_begin(position2, position), SampleBuffers::range_buffers_end());
            }

            sample.data.swap(data);
        }

    } // namespace mux
} // namespace ppbox
