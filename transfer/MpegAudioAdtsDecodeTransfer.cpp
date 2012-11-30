// MpegAudioAdtsDecodeTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/MpegAudioAdtsDecodeTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"

#include <ppbox/avformat/codec/aac/AacCodec.h>
#include <ppbox/avformat/codec/aac/AacAdts.h>
#include <ppbox/avformat/stream/BitsIStream.h>
#include <ppbox/avformat/stream/BitsBuffer.h>
using namespace ppbox::avformat;

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

            MyBuffersLimit limit(sample.data.begin(), sample.data.end());
            MyBuffersPosition position(limit);

            std::deque<boost::asio::const_buffer> data;

            while (buf.in_avail()) {
                AacAdts adts;
                bits_is >> adts;
                size_t adts_size = buf.in_position() - position.skipped_bytes();
                sample.size -= adts_size;
                MyBuffersPosition position2(position);
                position2.increment_bytes(limit, adts_size);;
                position.increment_bytes(limit, adts.frame_length);
                bits_is.seekg((size_t)adts.frame_length - adts_size, std::ios::cur);
                MyBufferIterator iter(limit, position2, position);
                MyBufferIterator end;
                data.insert(data.end(), iter, end);
            }

            sample.data.swap(data);
        }

    } // namespace mux
} // namespace ppbox
