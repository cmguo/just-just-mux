// StreamJoinTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/StreamJoinTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"
#include "ppbox/mux/elements.h"

#include <ppbox/avformat/codec/AvcConfig.h>
#include <ppbox/avformat/codec/AvcType.h>
#include <ppbox/avformat/BitsIStream.h>
#include <ppbox/avformat/BitsOStream.h>
#include <ppbox/avformat/BitsBuffer.h>

#include <util/archive/ArchiveBuffer.h>
#include <util/buffers/CycleBuffers.h>

#include <framework/system/BytesOrder.h>

namespace ppbox
{
    namespace mux
    {

        std::map<boost::uint32_t, ppbox::avformat::SeqParameterSetRbsp> spss;
        std::map<boost::uint32_t, ppbox::avformat::PicParameterSetRbsp> ppss;

        StreamJoinTransfer::StreamJoinTransfer()
            : sample_description_index_(boost::uint32_t(-1))
        {
            // start code
            nalu_start_code_.push_back(0);
            nalu_start_code_.push_back(0);
            nalu_start_code_.push_back(0);
            nalu_start_code_.push_back(1);
        }

        StreamJoinTransfer::~StreamJoinTransfer()
        {
        }

        void StreamJoinTransfer::transfer(
            ppbox::mux::MediaInfoEx & media)
        {
            ppbox::avformat::AvcConfig const & avc_config = 
                *(ppbox::avformat::AvcConfig const *)media.config;;
            // access unit delimiter
            access_unit_delimiter_ = nalu_start_code_;
            access_unit_delimiter_.push_back(9);
            access_unit_delimiter_.push_back(0xF0);
            // sps
            sps_pps_.clear();
            for (boost::uint32_t i = 0; i < avc_config.sequence_parameters().size(); i++) {
                std::vector<boost::uint8_t> sps_vec = avc_config.sequence_parameters()[i];
                //util::archive::ArchiveBuffer<boost::uint8_t> buf((boost::uint8_t *)&sps_vec[0], sps_vec.size(), sps_vec.size());
                //ppbox::avformat::BitsBuffer<boost::uint8_t> bits_buf(buf);
                //ppbox::avformat::BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                //ppbox::avformat::SeqParameterSetRbsp sps;
                //bits_reader >> sps;
                ////if (!sps.vui_parameters.fixed_frame_rate_flag) {
                ////    sps.vui_parameters.num_units_in_tick = 1;
                ////    sps.vui_parameters.time_scale = 50;
                ////    sps.vui_parameters.fixed_frame_rate_flag = 1;
                ////    sps_vec.resize(sps_vec.size() + 4);
                ////    util::archive::ArchiveBuffer<boost::uint8_t> buf((boost::uint8_t *)&sps_vec[0], sps_vec.size());
                ////    ppbox::avformat::BitsBuffer<boost::uint8_t> bits_buf(buf);
                ////    ppbox::avformat::BitsOStream<boost::uint8_t> bits_writer(bits_buf);
                ////    bits_writer << sps;
                ////    sps_vec.resize(buf.size());
                ////    
                ////}
                //spss.insert(std::make_pair(sps.sps_seq_parameter_set_id, sps));
                sps_pps_.insert(sps_pps_.end(), nalu_start_code_.begin(), nalu_start_code_.end());
                sps_pps_.insert(sps_pps_.end(), sps_vec.begin(), sps_vec.end());
            }
            // pps
            for (boost::uint32_t i = 0; i < avc_config.picture_parameters().size(); i++) {
                std::vector<boost::uint8_t> const & pps_vec = avc_config.picture_parameters()[i];
                //util::archive::ArchiveBuffer<boost::uint8_t> buf((boost::uint8_t *)&pps_vec[0], pps_vec.size(), pps_vec.size());
                //ppbox::avformat::BitsBuffer<boost::uint8_t> bits_buf(buf);
                //ppbox::avformat::BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                //ppbox::avformat::PicParameterSetRbsp pps(spss);
                //bits_reader >> pps;
                //ppss.insert(std::make_pair(pps.pps_pic_parameter_set_id, pps));
                sps_pps_.insert(sps_pps_.end(), nalu_start_code_.begin(), nalu_start_code_.end());
                sps_pps_.insert(sps_pps_.end(), pps_vec.begin(), pps_vec.end());
            }
        }

        void StreamJoinTransfer::transfer(
            ppbox::demux::Sample & sample)
        {
            //packet -> es
            if (sample.idesc != sample_description_index_) {
                sample_description_index_ = sample.idesc;
                NaluList const & nalus = 
                    *(NaluList const * )sample.context;
                bool need_header = true;
                for (boost::uint32_t i = 0; i < nalus.size(); ++i) {
                    Nal_header nalu_header = *(Nal_header*)&nalus[i].begin.dereference_byte();
                    if (nalu_header.nal_unit_type == NALUType::SPS) {
                        need_header = false;
                        break;
                    }
                }
                if (need_header) {
                    std::cout << "need_header" << std::endl;
                } else {
                    access_unit_delimiter_.clear();
                    sps_pps_.clear();
                }
            }

            std::deque<boost::asio::const_buffer> datas;
            sample.size = 0;
            datas.push_back(boost::asio::buffer(access_unit_delimiter_));
            sample.size += access_unit_delimiter_.size();
            //if (sample.flags & demux::Sample::sync) { 
            //    datas.push_back(boost::asio::buffer(sps_pps_));
            //    sample.size += sps_pps_.size();
            //}
            NaluList const & nalus = 
                *(NaluList const * )sample.context;
            util::buffers::BuffersLimit<ConstBuffers::const_iterator> limit(sample.data.begin(), sample.data.end());
            MyBufferIterator end;
            for (boost::uint32_t i = 0; i < nalus.size(); ++i) {
                Nal_header nalu_header = *(Nal_header*)&nalus[i].begin.dereference_byte();
                MyBufferIterator iter(limit, nalus[i].begin, nalus[i].end);
                //if (nalu_header.nal_unit_type == NALUType::AccessUnitDelimiter) {
                //    continue;
                //}
                //if (nalu_header.nal_unit_type == NALUType::SPS) {
                //    continue;
                //}
                //if (nalu_header.nal_unit_type == NALUType::PPS) {
                //    continue;
                //}
                //if (nalu_header.nal_unit_type == NALUType::SEI) {
                //    continue;
                //}
                if (nalu_header.nal_unit_type == NALUType::IDR) {
                    datas.push_back(boost::asio::buffer(sps_pps_));
                    sample.size += sps_pps_.size();
                }
                //if (nalu_header.nal_unit_type == NALUType::SEI) {
                //    std::cout << "Sei" << std::endl;
                //    MyBuffers buffers(iter);
                //    util::buffers::CycleBuffers<MyBuffers, boost::uint8_t> buf(buffers);
                //    buf.commit(nalus[i].size);
                //    ppbox::avformat::BitsBuffer<boost::uint8_t> bits_buf(buf);
                //    ppbox::avformat::BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                //    ppbox::avformat::SeiRbsp sei(spss, ppss);
                //    bits_reader >> sei;
                //}
                //if (nalu_header.nal_unit_type == NALUType::UNIDR
                //    || nalu_header.nal_unit_type == NALUType::IDR) {
                //        MyBuffers buffers(iter);
                //        util::buffers::CycleBuffers<MyBuffers, boost::uint8_t> buf(buffers);
                //        buf.commit(nalus[i].size);
                //        ppbox::avformat::BitsBuffer<boost::uint8_t> bits_buf(buf);
                //        ppbox::avformat::BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                //        ppbox::avformat::SliceLayerWithoutPartitioningRbsp slice(ppss);
                //        bits_reader >> slice;
                //        std::cout << "Frame type: " 
                //            << ppbox::avformat::SliceHeader::slice_type_str[slice.slice_header.slice_type] 
                //            << ", dts: " << sample.dts
                //            << ",\t cts: " << sample.dts + sample.cts_delta
                //            << std::endl;
                //}
                datas.push_back(boost::asio::buffer(nalu_start_code_));
                datas.insert(datas.end(), iter, end);
                sample.size += nalus[i].size + 4;
            }
            sample.data.swap(datas);
        }

    }
}
