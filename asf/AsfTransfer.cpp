// AsfTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/asf/AsfTransfer.h"
#include "ppbox/mux/MuxerBase.h"

#include <ppbox/avformat/asf/AsfObjectType.h>
#include <ppbox/avformat/asf/AsfFormat.h>
using namespace ppbox::avformat;

#include <ppbox/avbase/stream/SampleBuffers.h>
using namespace ppbox::avbase;

#include <util/archive/ArchiveBuffer.h>
#include <util/buffers/BuffersCopy.h>

#include <boost/asio/streambuf.hpp>

namespace ppbox
{
    namespace mux
    {

        size_t const PACKET_HEAD_LENGTH = 14;
        size_t const PAYLOAD_HEAD_LENGTH = 17;

        AsfTransfer::AsfTransfer()
            : single_payload_(false)
            , packet_length_(4096)
            , p_index_(0)
            , packet_left_(0)
            , data_ptr_(NULL)
        {
            data_buf_[0] = data_buf_[1] = 0;

            context_.packet = &packet_head_;
        }

        AsfTransfer::~AsfTransfer()
        {
            if (data_ptr_[0])
                delete [] data_buf_[0];
            if (data_ptr_[1])
                delete [] data_buf_[1];
        }

        void AsfTransfer::config(
            framework::configure::Config & conf)
        {
            if (data_ptr_)
                return;

            conf.register_module("Asf")
                << CONFIG_PARAM_NAME_RDWR("packet_length", packet_length_)
                << CONFIG_PARAM_NAME_RDWR("single_payload", single_payload_);

            context_.max_packet_size = packet_length_;

            data_buf_[0] = new boost::uint8_t[packet_length_];
            data_buf_[1] = new boost::uint8_t[packet_length_];
            data_ptr_ = data_buf_[0];
            pad_buf_.prepare(packet_length_ - PAYLOAD_HEAD_LENGTH - PACKET_HEAD_LENGTH);
            pad_buf_.commit(packet_length_ - PAYLOAD_HEAD_LENGTH - PACKET_HEAD_LENGTH);

            Sample sample;
            sample.time = 0;
            add_packet(sample, false);
        }

        void AsfTransfer::transfer(
            StreamInfo & info) 
        {
            media_number_.push_back(0);
        }

        void AsfTransfer::transfer(
            Sample & sample) 
        {
            SampleBuffers::BuffersPosition pos1(sample.data.begin(), sample.data.end());
            SampleBuffers::BuffersPosition end(sample.data.end());
            boost::uint32_t sample_remain = sample.size;

            packets_[0] = packets_.back();
            packets_.erase(packets_.begin() + 1, packets_.end());

            while (0 != sample_remain) {
                if(sample_remain + PAYLOAD_HEAD_LENGTH * 2 < packet_left_) {
                    SampleBuffers::BuffersPosition pos2 = pos1;
                    pos1.increment_bytes(end, sample_remain);
                    if (single_payload_) {
                        add_payload(sample, SampleBuffers::range_buffers(pos2, pos1), 
                            pos2.skipped_bytes(), pos1.skipped_bytes() - pos2.skipped_bytes(), 
                            (boost::uint8_t)(packet_left_ - (sample_remain + PAYLOAD_HEAD_LENGTH)), false);
                        add_packet(sample, false);
                    } else {
                        add_payload(sample, SampleBuffers::range_buffers(pos2, pos1), 
                            pos2.skipped_bytes(), pos1.skipped_bytes() - pos2.skipped_bytes(), 
                            0, true);
                    }
                    sample_remain = 0;
                } else if(sample_remain + PAYLOAD_HEAD_LENGTH < packet_left_) {
                    SampleBuffers::BuffersPosition pos2 = pos1;
                    pos1.increment_bytes(end, sample_remain);
                    add_payload(sample, SampleBuffers::range_buffers(pos2, pos1), 
                        pos2.skipped_bytes(), pos1.skipped_bytes() - pos2.skipped_bytes(), 
                        (boost::uint8_t)(packet_left_ - (sample_remain + PAYLOAD_HEAD_LENGTH)), false);
                    sample_remain = 0;
                    assert(packet_left_ == 0);
                    add_packet(sample, true);
                } else {
                    SampleBuffers::BuffersPosition pos2 = pos1;
                    pos1.increment_bytes(end, packet_left_ - PAYLOAD_HEAD_LENGTH);
                    sample_remain -= (packet_left_ - PAYLOAD_HEAD_LENGTH);
                    add_payload(sample, SampleBuffers::range_buffers(pos2, pos1), 
                        pos2.skipped_bytes(), pos1.skipped_bytes() - pos2.skipped_bytes(), 
                        0, false);
                    assert(packet_left_ == 0);
                    add_packet(sample, sample_remain < (packet_length_ - PACKET_HEAD_LENGTH - PAYLOAD_HEAD_LENGTH * 2));
                }
            }//while

            if(!single_payload_)
                assert(packet_left_ > PAYLOAD_HEAD_LENGTH);

            media_number_[sample.itrack]++;
            sample.data.clear();
            sample.data.insert(sample.data.end(), data_.begin(), data_.begin() + p_index_);
            data_.erase(data_.begin(), data_.begin() + p_index_);
            p_index_ = 0;
            head_buf_queue_.free_to_mark();

            sample.size = 
                single_payload_ ? packet_length_ : packet_length_ * (packets_.size() - 1);
            sample.context = &packets_;
            if (sample.data.empty()) {
                assert(sample.size == 0);
                sample.data.push_back(boost::asio::buffer(pad_buf_.data(), 0));
            }
        }

        void AsfTransfer::before_seek(
            Sample & sample)
        {
        }

        void AsfTransfer::on_seek(
            boost::uint64_t time)
        {
            data_.clear();
            packets_.clear();
            head_buf_queue_.free_all();

            Sample sample;
            sample.time = time;
            add_packet(sample, false);
        }

        void AsfTransfer::file_header(
            MediaInfo const & info, 
            size_t stream_obj_size, 
            boost::asio::streambuf & buf)
        {
            util::archive::LittleEndianBinaryOArchive<> oa(buf);

            //Header_Object
            // 先在buf中为header_object占位
            AsfHeaderObject header_object;
            oa << header_object;

            //ASF_File_Properties_Object
            AsfFilePropertiesObject file_object;
            file_object.ObjLength = 104;
            file_object.FileId.generate();
            file_object.FileSize = 104;
            file_object.PlayDuration = info.duration;
            file_object.Flag.BroadcastFlag = 1;
            //file_object.Flag.SeekableFlag = 0;
            file_object.Flag.Reserved = 1;
            file_object.MinimumDataPacketSize = packet_length();
            file_object.MaximumDataPacketSize = packet_length();
            oa << file_object;

            //ASF_Header_Extension_Object
            AsfHeaderExtensionObject extension_object;
            extension_object.ObjectSize = 46;
            //extension_object.HeaderExtensionDataSize = 0;
            oa << extension_object;

            // 重新计算header_object的字段
            header_object.ObjLength = buf.size() + stream_obj_size;
            header_object.NumOfHeaderObject = media_number_.size() + 2;
            util::archive::ArchiveBuffer<> abuf((char *)boost::asio::buffer_cast<char const *>(buf.data()), buf.size());
            util::archive::LittleEndianBinaryOArchive<> oa1(abuf);
            oa1 << header_object;
        }

        void AsfTransfer::data_header(
            boost::asio::streambuf & buf)
        {
            //Data_Object_header
            AsfDataObject data_object;
            data_object.ObjLength = 0x32;
            data_object.FileId.generate();
            //data_object.TotalDataPackets = 1;
            util::archive::LittleEndianBinaryOArchive<> oa(buf);
            oa << data_object;
        }

        void AsfTransfer::stream_header(
            StreamInfo const & info, 
            boost::asio::streambuf & buf)
        {
            AsfFormat asf;
            boost::system::error_code ec;
            CodecInfo const * codec = asf.codec_from_codec(info.type, info.sub_type, ec);
            if (codec == NULL) {
                return;
            }
            //structure ASF_Stream_Properties_Object
            AsfStreamPropertiesObject streams_object;
            streams_object.StreamType = 
                info.type == StreamType::VIDE ? ASF_Video_Media : ASF_Audio_Media;
            streams_object.ErrorCorrectionType = ASF_No_Error_Correction;
            streams_object.Flag.StreamNumber = info.index + 1;

            if (ASF_Video_Media == streams_object.StreamType) {
                streams_object.Video_Media_Type.EncodeImageWidth  = info.video_format.width;
                streams_object.Video_Media_Type.EncodeImageHeight = info.video_format.height;
                streams_object.Video_Media_Type.FormatData.FormatDataSize = 
                    streams_object.Video_Media_Type.FormatData.CodecSpecificData.size() + 40;
                streams_object.Video_Media_Type.FormatData.ImageWidth  = info.video_format.width;
                streams_object.Video_Media_Type.FormatData.ImageHeight = info.video_format.height;
                streams_object.Video_Media_Type.FormatData.BitsPerPixelCount = 24;
                streams_object.Video_Media_Type.FormatData.CompressionID = codec->stream_type;
                streams_object.Video_Media_Type.FormatData.CodecSpecificData = info.format_data;
                streams_object.Video_Media_Type.FormatDataSize = 
                    (boost::uint16_t)streams_object.Video_Media_Type.FormatData.FormatDataSize;
                streams_object.ObjLength = 89 + streams_object.Video_Media_Type.FormatDataSize;
                streams_object.TypeSpecificDataLength =
                    11 + streams_object.Video_Media_Type.FormatDataSize;
            } else {
                streams_object.Audio_Media_Type.CodecId = (boost::uint16_t)codec->stream_type;
                streams_object.Audio_Media_Type.NumberOfChannels = (boost::uint16_t)info.audio_format.channel_count;
                streams_object.Audio_Media_Type.SamplesPerSecond = info.audio_format.sample_rate;
                //streams_object.Audio_Media_Type.AverageNumberOfBytesPerSecond = 4000;
                //streams_object.Audio_Media_Type.BlockAlignment = 
                //    info.audio_format.channel_count * info.audio_format.sample_size / 8;
                //streams_object.Audio_Media_Type.BitsPerSample = info.audio_format.sample_size;
                streams_object.Audio_Media_Type.CodecSpecificDataSize = 
                    info.format_data.size();
                streams_object.Audio_Media_Type.CodecSpecificData = info.format_data;

                streams_object.ObjLength = 96 + streams_object.Audio_Media_Type.CodecSpecificDataSize;

                streams_object.TypeSpecificDataLength = 
                    18 + streams_object.Audio_Media_Type.CodecSpecificDataSize;
            }

            util::archive::LittleEndianBinaryOArchive<> oa(buf);
            oa << streams_object;
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
            packet_head_.PayLoadParseInfo.SendTime = (boost::uint32_t)sample.time + 2000;
            //packet_head_.PayLoadParseInfo.Duration = sample.duration;
            //packet_head_.PayloadNum = 0;
            packet_head_.PayloadLengthType = 2;

            head_buf_queue_.mark();
            boost::uint8_t * asf_buf = head_buf_queue_.alloc();
            FormatBuffer buf(asf_buf, PACKET_HEAD_LENGTH);
            AsfOArchive oar(buf);
            oar.context(&context_);
            oar << packet_head_;
            //填充到data_中
            p_index_ = data_.size();
            AsfPacket pkt;
            pkt.off_seg = p_index_;
            pkt.pad_len = 0;
            pkt.key_frame = false;
            packets_.push_back(pkt);
            data_.push_back(boost::asio::buffer(asf_buf, PACKET_HEAD_LENGTH));
        }

        template <typename BuffersContainer>
        void AsfTransfer::add_payload(//构造payload头部
            Sample & sample,
            BuffersContainer const & buffers,
            boost::uint32_t payload_offset, 
            boost::uint32_t payload_size, 
            boost::uint8_t padding_size,
            bool copy_flag)
        {
            AsfPayloadHeader payload_header;

            struct P_NUM {
#ifdef   BOOST_BIG_ENDIAN
                boost::uint8_t PayloadLengthType : 2;
                boost::uint8_t PayloadNum : 6;
#else
                boost::uint8_t PayloadNum : 6;
                boost::uint8_t PayloadLengthType : 2;
#endif
            } * p_pyd_num;

            p_pyd_num = reinterpret_cast<P_NUM *>(head_buf_queue_.get_mark() + PACKET_HEAD_LENGTH - 1);
            p_pyd_num->PayloadNum += 1;

            //payload头部构造
            payload_header.StreamNum = sample.itrack + 1;
            if( Sample::f_sync & sample.flags ) {
                payload_header.KeyFrameBit = 1;
                packets_.back().key_frame = true;
            } else {
                payload_header.KeyFrameBit = 0;
            }
            payload_header.MediaObjNum = media_number_[sample.itrack];
            payload_header.OffsetIntoMediaObj = payload_offset;
            payload_header.ReplicatedDataLen = 8;
            payload_header.MediaObjectSize = sample.size;
            payload_header.PresTime = (boost::uint32_t)sample.time + 2000;
            payload_header.PayloadLength = payload_size;

            boost::uint8_t * asf_buf = head_buf_queue_.alloc();
            FormatBuffer buf(asf_buf, PAYLOAD_HEAD_LENGTH);
            AsfOArchive oar(buf);
            oar.context(&context_);
            oar << payload_header;
            //填充到data_中
            data_.push_back(boost::asio::buffer(asf_buf, PAYLOAD_HEAD_LENGTH));

            //减少data的剩余空间
            assert(packet_left_ >= PAYLOAD_HEAD_LENGTH);
            packet_left_ -= PAYLOAD_HEAD_LENGTH;

            //若需要拷贝data
            if(copy_flag) {
                assert(packet_left_< packet_length_);
                util::buffers::buffers_copy(
                    boost::asio::buffer(data_ptr_ + packet_length_ - packet_left_, payload_size), 
                    buffers);
                data_.push_back(boost::asio::buffer(data_ptr_ + packet_length_ - packet_left_, payload_size));
            } else {
                data_.insert(data_.end(), buffers.begin(), buffers.end());
            }

            assert(packet_left_ >= payload_size);
            packet_left_ -= payload_size;

            if(0 != padding_size) {
                boost::uint8_t * pad_len = head_buf_queue_.get_mark() + PACKET_HEAD_LENGTH - 9;
                *pad_len = padding_size;
                assert(packet_left_ >= padding_size);
                packet_left_ -= padding_size;
                assert(packet_left_ == 0);
                data_.push_back(boost::asio::buffer(pad_buf_.data(), padding_size));
                packets_.back().pad_len = padding_size;
            }
        }

    } // namespace mux
} // namespace ppbox
