// StreamJoinTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/StreamJoinTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"

#include <ppbox/avformat/codec/AvcCodec.h>
#include <ppbox/avformat/codec/AvcNalu.h>

namespace ppbox
{
    namespace mux
    {

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
            ppbox::mux::StreamInfo & media)
        {
            ppbox::avformat::AvcConfig const & avc_config = 
                ((ppbox::avformat::AvcCodec const *)media.codec)->config();
            // access unit delimiter
            access_unit_delimiter_ = nalu_start_code_;
            access_unit_delimiter_.push_back(9);
            access_unit_delimiter_.push_back(0xF0);
            // sps
            sps_pps_.clear();
            for (boost::uint32_t i = 0; i < avc_config.sequence_parameters().size(); i++) {
                std::vector<boost::uint8_t> const & sps_vec = avc_config.sequence_parameters()[i];
                sps_pps_.insert(sps_pps_.end(), nalu_start_code_.begin(), nalu_start_code_.end());
                sps_pps_.insert(sps_pps_.end(), sps_vec.begin(), sps_vec.end());
            }
            // pps
            for (boost::uint32_t i = 0; i < avc_config.picture_parameters().size(); i++) {
                std::vector<boost::uint8_t> const & pps_vec = avc_config.picture_parameters()[i];
                sps_pps_.insert(sps_pps_.end(), nalu_start_code_.begin(), nalu_start_code_.end());
                sps_pps_.insert(sps_pps_.end(), pps_vec.begin(), pps_vec.end());
            }
        }

        void StreamJoinTransfer::transfer(
            Sample & sample)
        {
            //packet -> es
            if (sample.idesc != sample_description_index_) {
                sample_description_index_ = sample.idesc;
                NaluList const & nalus = 
                    *(NaluList const * )sample.context;
                bool need_aud = true;
                bool need_sps_pps = true;
                for (boost::uint32_t i = 0; i < nalus.size(); ++i) {
                    avformat::NaluHeader nalu_header(nalus[i].begin.dereference_byte());
                    if (nalu_header.nal_unit_type == avformat::NaluHeader::AccessUnitDelimiter) {
                        need_aud = false;
                    }
                    if (nalu_header.nal_unit_type == avformat::NaluHeader::SPS) {
                        need_sps_pps = false;
                    }
                }
                if (need_aud) {
                    std::cout << "need_aud" << std::endl;
                } else {
                    access_unit_delimiter_.clear();
                }
                if (need_sps_pps) {
                    std::cout << "need_sps_pps" << std::endl;
                } else {
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
                avformat::NaluHeader nalu_header(nalus[i].begin.dereference_byte());
                MyBufferIterator iter(limit, nalus[i].begin, nalus[i].end);
                //if (nalu_header.nal_unit_type == avformat::NaluHeader::AccessUnitDelimiter) {
                //    continue;
                //}
                //if (nalu_header.nal_unit_type == avformat::NaluHeader::SPS) {
                //    continue;
                //}
                //if (nalu_header.nal_unit_type == avformat::NaluHeader::PPS) {
                //    continue;
                //}
                //if (nalu_header.nal_unit_type == avformat::NaluHeader::SEI) {
                //    continue;
                //}
                if (nalu_header.nal_unit_type == avformat::NaluHeader::IDR) {
                    datas.push_back(boost::asio::buffer(sps_pps_));
                    sample.size += sps_pps_.size();
                }
                datas.push_back(boost::asio::buffer(nalu_start_code_));
                datas.insert(datas.end(), iter, end);
                sample.size += nalus[i].size + 4;
            }
            sample.data.swap(datas);
        }

    } // namespace mux
} // namespace ppbox
