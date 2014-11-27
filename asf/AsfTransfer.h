
#ifndef _JUST_MUX_ASF_ASF_TRANSFER_H_
#define _JUST_MUX_ASF_ASF_TRANSFER_H_

#include "just/mux/Transfer.h"
#include "just/mux/asf/AsfBuffer.h"

#include <just/avformat/asf/AsfObjectType.h>

#include <boost/asio/streambuf.hpp>

namespace just
{
    namespace mux
    {

        class MuxerBase;

        class AsfTransfer
            : public Transfer
        {
        public:
            AsfTransfer();

            ~AsfTransfer();

        public:
            virtual void config(
                framework::configure::Config & conf);

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

            virtual void before_seek(
                Sample & sample);

            virtual void on_seek(
                boost::uint64_t time);

        public:
            boost::uint32_t packet_length() const
            {
                return packet_length_;
            }

            void file_header(
                MediaInfo const & info, 
                size_t stream_obj_size, 
                boost::asio::streambuf & buf);

            void stream_header(
                StreamInfo const & info, 
                boost::asio::streambuf & buf);

            void data_header(
                boost::asio::streambuf & buf);

        public:
            struct AsfPacket
            {
                 size_t off_seg;
                 boost::uint8_t pad_len;
                 bool key_frame;
            };

        private:
            void add_packet(
                Sample & sample, 
                bool need_data_buf);

            template <typename BuffersContainer>
            void add_payload(
                Sample & sample,
                BuffersContainer const & buffers, 
                boost::uint32_t payload_offset, 
                boost::uint32_t payload_size, 
                boost::uint8_t padding_size,
                bool copy_flag);

        private:
            bool single_payload_;
            boost::uint32_t packet_length_;

        private:
            std::vector<boost::uint8_t> media_number_;
            std::deque<boost::asio::const_buffer> data_;//����buffer�ĵ�ַ�ͳ��ȼ�¼
            size_t p_index_;                            //���浱ǰ��packet��data_���±�ֵ
            std::vector<AsfPacket> packets_;
            just::avformat::AsfParseContext context_;
            just::avformat::AsfPacket packet_head_;
            boost::uint32_t packet_left_;               //��ǰpacket�ڴ�ʣ��ռ�
            AsfBufferQueue head_buf_queue_;
            boost::uint8_t * data_buf_[2];
            boost::uint8_t * data_ptr_;                 //ָ��ǰʹ�õ�cpoy data�ڴ�
            boost::asio::streambuf pad_buf_;
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_ASF_ASF_TRANSFER_H_
