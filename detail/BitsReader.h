// BitsReader.h

#ifndef _PPBOX_MUX_DETAIL_BITS_READER_H_
#define _PPBOX_MUX_DETAIL_BITS_READER_H_

#include <util/buffers/BuffersFind.h>

#include <util/buffers/BufferSize.h>

namespace ppbox
{
    namespace mux
    {

        template <
            typename ByteIterator
        >
        class BitsReader
        {
        public:
            BitsReader(
                ByteIterator const & iter,
                boost::uint32_t size)
                : iter_(iter)
                , size_(size)
                , size_this_byte_(0)
                , mask_this_byte_(0)
                , failed_(false)
            {
                next_byte();
            }

            boost::uint32_t read_bits_flc(
                boost::uint32_t len)
            {
                if (failed_) {
                    return 0;
                }
                boost::uint32_t v = 0;
                while (len > size_this_byte_) {
                    v = (v << size_this_byte_) | (((boost::uint32_t)(this_byte_)) & mask_this_byte_);
                    len -= size_this_byte_;
                    next_byte();
                    if (failed_) {
                        return 0;
                    }
                }
                if (len) {
                    v = (v << len) | ((((boost::uint32_t)(this_byte_)) >> (size_this_byte_ - len)) & ((1 << len) - 1));
                    size_this_byte_ -= len;
                    mask_this_byte_ >>= len;
                }
                return v;
            }

            boost::uint32_t read_bits_vlc(
                boost::uint32_t & len)
            {
                if (failed_) {
                    return 0;
                }
                boost::uint32_t v = ((boost::uint32_t)(this_byte_)) & mask_this_byte_;
                len = 0;
                while (true) {
                    size_t n = 0;
                    while (v) {
                        ++n;
                        v >>= 1;
                    }
                    if (n) {
                        n = size_this_byte_ - n;
                        len += n;
                        size_this_byte_ -= n;
                        mask_this_byte_ >>= n;
                        break;
                    } else {
                        len += size_this_byte_;
                        next_byte();
                        if (failed_) {
                            return 0;
                        }
                        v = (boost::uint32_t)(this_byte_);
                    }
                }
                boost::uint32_t res = read_bits_flc(len+1);
                boost::uint32_t i = len;
                boost::uint32_t mask = 0xFFFFFFFF;
                mask <<= i;
                mask = ~mask;
                res &= mask;
                len = len*2 + 1;
                return res;
            }
            
            bool failed() const
            {
                return failed_;
            }

        private:
            void next_byte()
            {
                if (iter_ == ByteIterator()) {
                    failed_ = true;
                    return;
                }
                this_byte_ = *(iter_++);
                size_this_byte_ = 8;
                mask_this_byte_ = 0xff;
                this_byte_ = this_byte_ & mask_this_byte_;
                //++iter_;
                --size_;
            }
            
        private:
            ByteIterator iter_;
            boost::uint32_t size_;
            boost::uint32_t this_byte_;
            boost::uint32_t size_this_byte_;
            boost::uint32_t mask_this_byte_;
            bool failed_;
        };

        typedef std::deque<boost::asio::const_buffer> ConstBuffers;

        typedef util::buffers::BuffersLimit<ConstBuffers::const_iterator> MyBuffersLimit;

        typedef util::buffers::BuffersPosition<
            ConstBuffers::value_type, 
            ConstBuffers::const_iterator
        > MyBuffersPosition;

        typedef util::buffers::BuffersFindIterator2<
            ConstBuffers, 
            boost::asio::const_buffers_1
        > MyFindIterator2;

        typedef MyFindIterator2::ByteIterator MyByteIterator;

        typedef MyFindIterator2::BuffersIterator MyBufferIterator;

        typedef BitsReader<MyByteIterator> MyBitsReader;

        typedef util::buffers::Container<boost::asio::const_buffer, MyBufferIterator> MyBuffers;

        struct Nalu
        {
            Nalu(
                size_t t,
                MyBuffersPosition const & b,
                MyBuffersPosition const & e)
                : size(t)
                , begin(b)
                , end(e)
            {
            }

            MyBuffers buffers(MyBuffersLimit const & limit)
            {
                return MyBuffers(MyBufferIterator(limit, begin, end));
            }

            size_t size;
            MyBuffersPosition begin;
            MyBuffersPosition end;
        };

        struct NaluList
            : std::vector<Nalu>
        {
            size_t total_size;
        };

    }
}

#endif // _PPBOX_MUX_DETAIL_BITS_READER_H_
