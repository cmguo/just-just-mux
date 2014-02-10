// Mp4DataContext.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/mp4/Mp4DataContext.h"

#include <ppbox/avformat/mp4/box/Mp4BoxEnum.h>
#include <ppbox/avformat/mp4/box/Mp4BoxHeader.h>
#include <ppbox/avformat/mp4/box/Mp4BoxArchive.h>
using namespace ppbox::avformat;

#include <util/buffers/BuffersCopy.h>
#include <util/archive/ArchiveBuffer.h>

namespace ppbox
{
    namespace mux
    {

        Mp4DataContext::Mp4DataContext(
            boost::uint32_t block_size)
            : block_size_(block_size)
            , offset_(0)
            , block_end_(block_size)
        {
            Mp4BoxHeader mdat;
            mdat.type = Mp4BoxType::mdat;
            mdat.largesize = mdat.size = block_size_;
            mdat_.resize(mdat.head_size());
            util::archive::ArchiveBuffer<boost::uint8_t> buf(boost::asio::buffer(mdat_));
            Mp4BoxOArchive oa(buf);
            oa << mdat;

            padding_.resize(1024);
        }

        void Mp4DataContext::put_header(
            Sample & sample)
        {
            Mp4BoxHeader mdat;
            mdat.type = Mp4BoxType::mdat;
            mdat.data_size(block_size_ - sample.size);
            util::archive::ArchiveBuffer<boost::uint8_t> buf(sample.data.front());
            buf.pubseekpos(sample.size - mdat.head_size(), std::ios::out);
            Mp4BoxOArchive oa(buf);
            oa << mdat;
            offset_ = sample.size;
        }

        void Mp4DataContext::put_sample(
            Mp4SampleTable & table_, 
            Sample & sample)
        {
            Sample sample2;
            if (offset_ + sample.size > block_end_) {
                next_block(sample2);
            }
            sample.time = offset_;
            offset_ += sample.size;
            assert(offset_ <= block_end_);
            table_.put(sample);
            if (sample2.size) {
                sample.size += sample2.size;
                sample2.data.insert(sample2.data.end(), sample.data.begin(), sample.data.end());
                sample.data.swap(sample2.data);
            }
        }

        void Mp4DataContext::pad_block(
            Sample & sample)
        {
            size_t pad_size = (size_t)(block_end_ - offset_);
            sample.size += (boost::uint32_t)pad_size;
            while (pad_size > padding_.size()) {
                sample.data.push_back(boost::asio::buffer(padding_));
                pad_size -= padding_.size();
            }
            sample.data.push_back(boost::asio::buffer(padding_, pad_size));
            offset_ = block_end_;
            block_end_ += block_size_;
        }

        void Mp4DataContext::next_block(
            Sample & sample)
        {
            pad_block(sample);
            sample.data.push_back(boost::asio::buffer(mdat_));
            sample.size += mdat_.size();
            offset_ += mdat_.size();
        }

    } // namespace mux
} // namespace ppbox
