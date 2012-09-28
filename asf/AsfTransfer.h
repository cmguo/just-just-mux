
#ifndef _PPBOX_MUX_ASF_TRANSFER_H_
#define _PPBOX_MUX_ASF_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"
#include <ppbox/mux/detail/BitsReader.h>

#include <ppbox/avformat/asf/AsfObjectType.h>

#include <boost/asio/streambuf.hpp>

namespace ppbox
{
    namespace mux
    {

        class AsfTransfer
            : public Transfer
        {
        public:
            AsfTransfer(
                MuxerBase & muxer);

            ~AsfTransfer();

        public:
            virtual void transfer(
                ppbox::demux::StreamInfo & mediainfo);

            virtual void transfer(
                ppbox::demux::Sample & sample);

        public:
            void on_seek(
                boost::uint64_t time);

            boost::uint32_t packet_length() const
            {
                return packet_length_;
            }	

        public:
            struct AsfPacket
            {
                 size_t off_seg;
                 size_t pad_len;
                 bool key_frame;
            };

        private:
            void add_packet(
                ppbox::demux::Sample & sample, 
                bool need_data_buf);

            void add_payload(
                ppbox::demux::Sample & sample,
                MyBuffersLimit & limit,
                MyBuffersPosition & pos1,
                MyBuffersPosition & pos2,
                size_t padding_size,
                bool copy_flag);

        private:
            struct HeadBlock
            {
                union {
                    boost::uint8_t buf[19];
                    HeadBlock * next_seg;
                };
                boost::uint8_t index;
            };

            struct HeadBlockPointer
            {
            public:
                HeadBlockPointer()
                    : ptr_(NULL)
                {
                }

                HeadBlockPointer(
                    HeadBlock * ptr)
                    : ptr_(ptr)
                {
                }

                HeadBlockPointer & operator+(int n)
                {
                    ptr_ += n;
                    return *this;
                }

                HeadBlock * operator->()
                {
                    return ptr_;
                }

                HeadBlock & operator*()
                {
                    return *ptr_;
                }

                HeadBlockPointer & operator++(int)
                {
                    ptr_++;
                    if(255 == ptr_->index)
                        ptr_ = ptr_->next_seg;
                    return *this;
                }

                HeadBlockPointer & operator=(
                    HeadBlockPointer const & s)
                {
                    ptr_ = s.ptr_;
                    return *this;
                }

                friend bool operator==(
                    HeadBlockPointer const & l, 
                    HeadBlockPointer const & r)
                {
                    return (l.ptr_ == r.ptr_);
                }

                HeadBlockPointer & set_ptr(
                    HeadBlock * hb_ptr)
                {
                    ptr_ = hb_ptr;
                    return *this;
                }

            private:
                HeadBlock * ptr_;
            };

        public:
            boost::uint32_t packet_length_;
            bool single_payload_;

        private:
            std::deque<boost::asio::const_buffer> data_;//保存buffer的地址和长度记录
            size_t p_index_;                            //保存当前的packet在data_中下标值
            std::vector<AsfPacket> packets_;
            boost::uint32_t packet_left_;               //当前packet内存剩余空间
            HeadBlockPointer head_beg_;
            HeadBlockPointer head_end_;
            HeadBlockPointer head_pkt_;
            HeadBlockPointer head_last_ptr_;            //当前packet内存最后一个内存块
            boost::uint8_t * data_buf_[2];
            boost::uint8_t * data_ptr_;                 //指向当前使用的cpoy data内存
            boost::uint16_t packet_count_;              //记录当前Packet头部数目
            std::vector<boost::uint8_t> media_number_;
            boost::asio::streambuf pad_buf_;
            boost::uint32_t max_packet_length_;
            ppbox::avformat::ASF_Packet packet_head_;
            std::vector<HeadBlock*> head_buf_ptr_;
        };
    } 
}

#endif
