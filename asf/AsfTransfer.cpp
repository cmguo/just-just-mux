// AsfTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/asf/AsfTransfer.h"
#include "ppbox/mux/Muxer.h"

#include <ppbox/avformat/asf/AsfObjectType.h>

#include <util/archive/BigEndianBinaryOArchive.h>
#include <util/archive/ArchiveBuffer.h>
#include <util/buffers/BufferCopy.h>

#include <boost/asio/streambuf.hpp>

using namespace ppbox::demux;
using namespace ppbox::avformat;

size_t const PACKET_HEAD_LENGTH = 14;
size_t const PAYLOAD_HEAD_LENGTH = 17;

namespace ppbox
{
    namespace mux
    {
        AsfTransfer::AsfTransfer(
            Muxer & muxer)
            : packet_length_(4096)
            , single_payload_(false)
            , p_index_(0)
            , packet_left_(0)
            , head_beg_(new HeadBlock[256])
            , head_end_(head_beg_)
            , head_pkt_(NULL)
            , head_last_ptr_(NULL)
            , packet_count_(0)
            , media_number_(0)
            , packet_head_(max_packet_length_)
        {
            muxer.config().register_module("Asf")
                << CONFIG_PARAM_NAME_RDWR("packet_length", packet_length_)
                << CONFIG_PARAM_NAME_RDWR("single_payload", single_payload_);

            max_packet_length_ = packet_length_;

            data_buf_[0] = new boost::uint8_t[packet_length_];
            data_buf_[1] = new boost::uint8_t[packet_length_];
            data_ptr_ = data_buf_[0];
            pad_buf_.prepare(packet_length_ - PAYLOAD_HEAD_LENGTH - PACKET_HEAD_LENGTH);
            pad_buf_.commit(packet_length_ - PAYLOAD_HEAD_LENGTH - PACKET_HEAD_LENGTH);

            //最后一个HeadBlock保存当前内存的首地址
            HeadBlock * p = &(*head_beg_);
            HeadBlock * tmp = p;
            for(int i = 0; i <256 ; i++){
                tmp->index = i;
                if(255 != i)
                    tmp++;
            }
            p[255].next_seg = p;
            head_last_ptr_.set_ptr(p + 255);
            head_buf_ptr_.push_back(p);

            media_number_.resize(2);
            Sample sample;
            sample.time = 0;
            add_packet(sample, false);
        }

        AsfTransfer::~AsfTransfer()
        {
            std::vector<HeadBlock*>::iterator itr = head_buf_ptr_.begin();
            while(head_buf_ptr_.end() != itr) {
                delete [](*itr);
                itr++;
            }
            delete [] data_buf_[0];
            delete [] data_buf_[1];
        }

        void AsfTransfer::add_packet(//构造packet头部
            Sample & sample, 
            bool need_data_buf)
        {
            if(need_data_buf) {
                data_ptr_ = 
                    data_ptr_ == data_buf_[0] ? data_buf_[1] : data_buf_[0];
            }

            packet_left_ = packet_length_ - PACKET_HEAD_LENGTH;
            //设置packet头部.
            //ErrorCorrectionData
            packet_head_.ErrorCorrectionInfo.ErrorCorrectionDataLength = 2;
            packet_head_.ErrorCorrectionInfo.OpaqueDataPresent = 0;
            packet_head_.ErrorCorrectionInfo.ErrorCorrectionLengthType = 0;
            packet_head_.ErrorCorrectionInfo.ErrorCorrectionPresent = 1;
            packet_head_.ErrorCorrectionInfo.Data.insert(
                packet_head_.ErrorCorrectionInfo.Data.end(), 2, 0);
            //PayLoadParseInfo
            packet_head_.PayLoadParseInfo.MultiplePayloadsPresent = 1;
            //packet_head_.PayLoadParseInfo.SequenceType = 0;
            packet_head_.PayLoadParseInfo.PaddingLengthType = 2;
            packet_head_.PayLoadParseInfo.PacketLengthType = 0;
            //packet_head_.PayLoadParseInfo.ErrorCorrectionPresent = 0;
            packet_head_.PayLoadParseInfo.ReplicatedDataLengthType = 1;
            packet_head_.PayLoadParseInfo.OffsetIntoMOLType = 3;
            packet_head_.PayLoadParseInfo.MediaObjNumType = 1;
            packet_head_.PayLoadParseInfo.StreamNumLengthType = 1;
            //packet_head_.PayLoadParseInfo.PacketLenth = packet_length_;
            //packet_head_.PayLoadParseInfo.Sequence = 0;
            packet_head_.PayLoadParseInfo.PaddingLength = 0;
            packet_head_.PayLoadParseInfo.SendTime = sample.time + 2000;
            //packet_head_.PayLoadParseInfo.Duration = sample.duration;
            //packet_head_.PayloadNum = 0;
            packet_head_.PayloadLengthType = 2;

            util::archive::ArchiveBuffer<boost::uint8_t> buf(head_end_->buf, PACKET_HEAD_LENGTH);
            ASFOArchive oar(buf);
            oar << packet_head_;
            head_pkt_ = head_end_;
            //填充到data_中
            p_index_ = data_.size();
            AsfPacket pkt;
            pkt.off_seg = p_index_;
            pkt.pad_len = 0;
            pkt.key_frame = false;
            packets_.push_back(pkt);
            data_.push_back(boost::asio::buffer(head_end_->buf, PACKET_HEAD_LENGTH));
            head_end_++;

            //判断当前的head pool是否已满
            if(head_beg_ == head_end_) {
                HeadBlockPointer hbp(new HeadBlock[256]);
                HeadBlock * p = &(*hbp);
                HeadBlock * q = p;
                for(int i = 0; i <256 ; i++) {
                    q->index = i;
                    if(255 != i)
                        q++;
                }
                head_buf_ptr_.push_back(&(*hbp));

                p[255].next_seg = head_last_ptr_->next_seg;
                head_last_ptr_->next_seg = p;
                head_last_ptr_.set_ptr(p + 255);

                //需要拷贝head_beg以后数据组合成循环队列
                int index = head_beg_->index;
                HeadBlock * src_ptr = &(*head_beg_);
                head_beg_.set_ptr(p + index);
                while(255 > index) {
                    p[index] = *src_ptr;
                    src_ptr++;
                    index++;
                }
            }

            packet_count_++;
        }

        void AsfTransfer::add_payload(//构造payload头部
            Sample & sample,
            MyBuffersLimit & limit,
            MyBuffersPosition & pos1,
            MyBuffersPosition & pos2,
            size_t padding_size,
            bool copy_flag)
        {
            ASF_PayloadHeader payload_header;
            payload_header.set_packet(packet_head_);

            struct P_NUM {
#ifdef   BOOST_BIG_ENDIAN
                boost::uint8_t PayloadLengthType : 2;
                boost::uint8_t PayloadNum : 6;
#else
                boost::uint8_t PayloadNum : 6;
                boost::uint8_t PayloadLengthType : 2;
#endif
            } * p_pyd_num;

            p_pyd_num = reinterpret_cast<P_NUM *>(head_pkt_->buf + PACKET_HEAD_LENGTH - 1);
            p_pyd_num->PayloadNum += 1;

            size_t payload_size = pos2.skipped_bytes() - pos1.skipped_bytes();
            //payload头部构造
            payload_header.StreamNum = sample.itrack + 1;
            if( Sample::sync & sample.flags ) {
                payload_header.KeyFrameBit = 1;
                packets_.back().key_frame = true;
            } else {
                payload_header.KeyFrameBit = 0;
            }
            payload_header.MediaObjNum = media_number_[sample.itrack];
            payload_header.OffsetIntoMediaObj = pos1.skipped_bytes();
            payload_header.ReplicatedDataLen = 8;
            payload_header.MediaObjectSize = sample.size;
            payload_header.PresTime = sample.time + 2000;
            payload_header.PayloadLength = payload_size;

            util::archive::ArchiveBuffer<boost::uint8_t> buf(head_end_->buf, PAYLOAD_HEAD_LENGTH);
            ASFOArchive oar(buf);
            oar << payload_header;
            //填充到data_中
            data_.push_back(boost::asio::buffer(head_end_->buf, PAYLOAD_HEAD_LENGTH));
            head_end_++;

            //判断当前的head pool是否已满
            if(head_beg_ == head_end_) {
                HeadBlockPointer hbp(new HeadBlock[256]);
                HeadBlock * p = &(*hbp);
                HeadBlock * q = p;
                for(int i = 0; i <256 ; i++) {
                    q->index = i;
                    if(255 != i)
                        q++;
                }
                head_buf_ptr_.push_back(&(*hbp));

                p[255].next_seg = head_last_ptr_->next_seg;
                head_last_ptr_->next_seg = p;
                head_last_ptr_.set_ptr(p + 255);

                //拷贝数据
                int index = head_beg_->index;
                HeadBlock * src_ptr = &(*head_beg_);
                head_beg_.set_ptr(p + index);
                while(255 > index) {
                    p[index] = *src_ptr;
                    src_ptr++;
                    index++;
                }
            }

            //减少data的剩余空间
            assert(packet_left_ >= PAYLOAD_HEAD_LENGTH);
            packet_left_ -= PAYLOAD_HEAD_LENGTH;

            //若需要拷贝data
            if(copy_flag) {
                assert(packet_left_< packet_length_);
                util::buffers::buffer_copy(
                    boost::asio::buffer(data_ptr_ + packet_length_ - packet_left_, payload_size), 
                    MyBuffers(MyBufferIterator(limit, pos1, pos2)));
                data_.push_back(boost::asio::buffer(data_ptr_ + packet_length_ - packet_left_, payload_size));
            } else {
                data_.insert(data_.end(), MyBufferIterator(limit, pos1, pos2), MyBufferIterator());
            }

            assert(packet_left_ >= payload_size);
            packet_left_ -= payload_size;

            if(0 != padding_size) {
                boost::uint8_t * pad_len = head_pkt_->buf + PACKET_HEAD_LENGTH - 9;
                *pad_len = padding_size;
                assert(packet_left_ >= padding_size);
                packet_left_ -= padding_size;
                assert(packet_left_ == 0);
                data_.push_back(boost::asio::buffer(pad_buf_.data(), padding_size));
                packets_.back().pad_len = padding_size;
            }
        }

        void AsfTransfer::transfer(
            StreamInfo & mediainfo) 
        {
            media_number_.push_back(0);
        }

        void AsfTransfer::transfer(
            Sample & sample) 
        {
            MyBuffersLimit limit(sample.data.begin(), sample.data.end());
            MyBuffersPosition pos1(limit);
            boost::uint32_t sample_remain = sample.size;

            packets_[0] = packets_.back();
            packets_.erase(packets_.begin() + 1, packets_.end());

            while (0 != sample_remain) {
                if(sample_remain + PAYLOAD_HEAD_LENGTH * 2 < packet_left_) {
                    MyBuffersPosition pos2 = pos1;
                    pos1.increment_bytes(limit, sample_remain);
                    if (single_payload_) {
                        add_payload(sample, limit, pos2, pos1,
                            packet_left_ - (sample_remain + PAYLOAD_HEAD_LENGTH), false);
                        add_packet(sample, false);
                    } else {
                        add_payload(sample, limit, pos2, pos1, 0, true);
                    }
                    sample_remain = 0;
                } else if(sample_remain + PAYLOAD_HEAD_LENGTH < packet_left_) {
                    MyBuffersPosition pos2 = pos1;
                    pos1.increment_bytes(limit, sample_remain);
                    add_payload(sample, limit, pos2, pos1, 
                        packet_left_ - (sample_remain + PAYLOAD_HEAD_LENGTH), false);
                    sample_remain = 0;
                    assert(packet_left_ == 0);
                    add_packet(sample, true);
                } else {
                    MyBuffersPosition pos2 = pos1;
                    pos1.increment_bytes(limit, packet_left_ - PAYLOAD_HEAD_LENGTH);
                    sample_remain -= (packet_left_ - PAYLOAD_HEAD_LENGTH);
                    add_payload(sample, limit, pos2, pos1, 0, false);
                    assert(packet_left_ == 0);
                    add_packet(sample, sample_remain < packet_length_ - PACKET_HEAD_LENGTH - PAYLOAD_HEAD_LENGTH * 2);
                }
            }//while

            if(!single_payload_)
                assert(packet_left_ > PAYLOAD_HEAD_LENGTH);

            media_number_[sample.itrack]++;
            sample.data.clear();
            sample.data.insert(sample.data.end(), data_.begin(), data_.begin() + p_index_);
            data_.erase(data_.begin(), data_.begin() + p_index_);
            p_index_ = 0;
            head_beg_ = head_pkt_;

            sample.size = 
                single_payload_ ? packet_length_ : packet_length_ * packet_count_;
            packet_count_ = 0;
            sample.context = &packets_;
            if (sample.data.empty()) {
                sample.data.push_back(boost::asio::buffer(pad_buf_.data(), 0));
            }
        }//AsfTransfer::transfer

        void AsfTransfer::on_seek(
            boost::uint32_t time)
        {
            packet_count_ = 0;
            data_.clear();
            packets_.clear();
            head_beg_ = head_end_;

            Sample sample;
            sample.time = time;
            add_packet(sample, false);
        }//on_seek

    }//mux
}//ppbox

