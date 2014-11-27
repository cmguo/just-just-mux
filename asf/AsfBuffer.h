
#ifndef _JUST_MUX_ASF_ASF_HEAD_BUFFER_H_
#define _JUST_MUX_ASF_ASF_HEAD_BUFFER_H_

namespace just
{
    namespace mux
    {

        struct AsfBuffer
        {
            AsfBuffer * prev;
            AsfBuffer * next;
            size_t block;
            size_t index;
            boost::uint8_t buf[20];
        };

        struct AsfBufferQueue
        {
        public:
            AsfBufferQueue()
                : begin_(NULL)
                , end_(NULL)
                , mark_(NULL)
            {
                alloc_block();
            }

            ~AsfBufferQueue()
            {
                for (size_t i = 0; i < blocks_.size(); ++i) {
                    delete [] blocks_[i];
                }
            }

        public:
            boost::uint8_t * alloc()
            {
                AsfBuffer * tmp = begin_;
                begin_ = begin_->next;
                if (begin_ == end_) {
                    alloc_block();
                }
                return tmp->buf;
            }

            void free()
            {
                assert(begin_ != end_);
                end_ = end_->next;
            }

            void mark()
            {
                mark_ = begin_;
            }

            boost::uint8_t * get_mark()
            {
                assert(mark_);
                return mark_->buf;
            }

            void free_to_mark()
            {
                assert(mark_);
                end_ = mark_;
            }

            void free_all()
            {
                end_ = begin_;
            }

            void dump() const
            {
                
            }

        private:
            void alloc_block()
            {
                AsfBuffer * blk = new AsfBuffer[256];
                size_t block = blocks_.size();
                for (size_t i = 0; i < 256; ++i) {
                    blk[i].next = blk + i + 1;
                    blk[i].prev = blk + i - 1;
                    blk[i].block = block;
                    blk[i].index = i;
                    if (begin_ == NULL) {
                        blk[255].next = blk;
                        blk[0].prev = blk + 255;
                        begin_ = end_ = blk;
                    } else {
                        blk[255].next = begin_;
                        blk[0].prev = begin_->prev;
                        begin_->prev = blk + 255;
                        begin_->prev->next = blk;
                        begin_ = blk;
                    }
                }
                blocks_.push_back(blk);
            }

        private:
            AsfBuffer * begin_;
            AsfBuffer * end_;
            AsfBuffer * mark_;
            std::vector<AsfBuffer *> blocks_;
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_ASF_ASF_HEAD_BUFFER_H_
