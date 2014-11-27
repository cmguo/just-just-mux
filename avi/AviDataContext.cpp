// AviDataContext.cpp

#include "just/mux/Common.h"
#include "just/mux/avi/AviDataContext.h"

#include <just/avformat/avi/box/AviBoxEnum.h>
#include <just/avformat/avi/box/AviBoxHeader.h>
#include <just/avformat/avi/box/AviBoxArchive.h>
using namespace just::avformat;

#include <util/buffers/BuffersCopy.h>
#include <util/archive/ArchiveBuffer.h>

namespace just
{
    namespace mux
    {

        AviDataContext::AviDataContext()
            : block_size_(0)
            , offset_(0)
            , block_end_(0)
        {
        }

        void AviDataContext::open(
            boost::uint32_t block_size)
        {
            block_size_ = block_size;
            block_end_ = block_size;

            AviBoxHeader movi;
            movi.list_id(AviBoxType::movi);
            movi.id(AviBoxType::JUNK);
            movi.byte_size(block_size_);
            movi_buf_.resize(movi.head_size());
            util::archive::ArchiveBuffer<boost::uint8_t> buf(boost::asio::buffer(movi_buf_));
            AviBoxOArchive oa(buf);
            oa << movi;

            padding_.resize(1024);
            memcpy(&padding_[0], "JUNK", 4);
        }

        void AviDataContext::put_header(
            Sample & sample)
        {
            util::archive::ArchiveBuffer<boost::uint8_t> buf(sample.data.front());
            AviBoxOArchive oa(buf);

            AviBoxHeader avi;
            avi.riff_id(AviBoxType::AVI);
            avi.data_size(0x70000000);
            oa.seekp(0);
            oa << avi;

            AviBoxHeader movi;
            movi.list_id(AviBoxType::movi);
            movi.data_size(block_size_ - sample.size);
            oa.seekp(sample.size - movi.head_size());
            oa << movi;

            offset_ = sample.size;
        }

        void AviDataContext::put_sample(
            AviStream & stream, 
            Sample & sample)
        {
            Sample sample2;
            boost::uint64_t offset = offset_ + sample.size;
            if (sample.size & 1) {
                ++offset;
            }
            if (offset + 8 > block_end_ && offset != block_end_) {
                next_block(sample2);
            }
            sample.time = offset_;
            stream.put(sample);
            if (sample.size & 1) {
                sample.data.push_back(boost::asio::buffer(padding_, 1));
                ++sample.size;
            }
            offset_ += sample.size;
            assert(offset_ <= block_end_);
            if (sample2.size) {
                sample.size += sample2.size;
                sample2.data.insert(sample2.data.end(), sample.data.begin(), sample.data.end());
                sample.data.swap(sample2.data);
            }
        }

        void AviDataContext::pad_block(
            Sample & sample)
        {
            size_t pad_size = (size_t)(block_end_ - offset_);
            assert(pad_size >= 8);
            sample.size += (boost::uint32_t)pad_size;
            boost::uint32_t size = framework::system::BytesOrder::host_to_little_endian(pad_size - 8);
            memcpy(&padding_[4], &size, 4);
            while (pad_size > padding_.size()) {
                sample.data.push_back(boost::asio::buffer(padding_));
                pad_size -= padding_.size();
            }
            sample.data.push_back(boost::asio::buffer(padding_, pad_size));
            offset_ = block_end_;
            block_end_ += block_size_;
        }

        void AviDataContext::next_block(
            Sample & sample)
        {
            pad_block(sample);
            sample.data.push_back(boost::asio::buffer(movi_buf_));
            sample.size += movi_buf_.size();
            offset_ += movi_buf_.size();
        }

    } // namespace mux
} // namespace just
