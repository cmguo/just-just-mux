// StreamJoinTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/StreamJoinTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"
#include "ppbox/mux/elements.h"

#include <ppbox/avformat/codec/AvcConfig.h>

#include <framework/system/BytesOrder.h>

namespace ppbox
{
    namespace mux
    {

        void StreamJoinTransfer::transfer(ppbox::demux::Sample & sample)
        {
            //packet -> es
            if (sample.idesc != sample_description_index_) {
                sample_description_index_ = sample.idesc;
                MediaInfoEx const * video_info = (MediaInfoEx const *)sample.media_info;
                ppbox::avformat::AvcConfig const & avc_config = 
                    *(ppbox::avformat::AvcConfig const *)video_info->config;;
                // start code
                nalu_start_code_.clear();
                nalu_start_code_.push_back(0);
                nalu_start_code_.push_back(0);
                nalu_start_code_.push_back(0);
                nalu_start_code_.push_back(1);
                // access unit delimiter
                access_unit_delimiter_.clear();
                access_unit_delimiter_ = nalu_start_code_;
                access_unit_delimiter_.push_back(9);
                access_unit_delimiter_.push_back(0xE0);
                // sps
                for (boost::uint32_t i = 0; i < avc_config.sequence_parameters().size(); i++) {
                    sps_pps_.insert(sps_pps_.end(), 
                        nalu_start_code_.begin(), nalu_start_code_.end());
                    sps_pps_.insert(sps_pps_.end(), 
                        avc_config.sequence_parameters()[i].begin(), 
                        avc_config.sequence_parameters()[i].end());
                }
                // pps
                for (boost::uint32_t i = 0; i < avc_config.picture_parameters().size(); i++) {
                    sps_pps_.insert(sps_pps_.end(), 
                        nalu_start_code_.begin(), nalu_start_code_.end());
                    sps_pps_.insert(sps_pps_.end(), 
                        avc_config.picture_parameters()[i].begin(), 
                        avc_config.picture_parameters()[i].end());
                }
            }

            std::deque<boost::asio::const_buffer> datas;
            sample.size = 0;
            if (sample.flags & demux::Sample::sync) {
                //datas.push_back(boost::asio::buffer(access_unit_delimiter_));
                //sample.size += access_unit_delimiter_.size();
                datas.push_back(boost::asio::buffer(sps_pps_));
                sample.size += sps_pps_.size();
            } else {
                //datas.push_back(boost::asio::buffer(access_unit_delimiter2_));
                //sample.size += access_unit_delimiter2_.size();
            }
            NaluList const & nalus = 
                *(NaluList const * )sample.context;
            util::buffers::BuffersLimit<ConstBuffers::const_iterator> limit(sample.data.begin(), sample.data.end());
            MyBufferIterator end;
            for (boost::uint32_t i = 0; i < nalus.size(); ++i) {
                Nal_header nalu_header = *(Nal_header*)&nalus[i].begin.dereference_byte();
                //if (nalu_header.nal_unit_type == NALUType::IDR 
                //    || nalu_header.nal_unit_type == NALUType::UNIDR) {
                        datas.push_back(boost::asio::buffer(nalu_start_code_));
                        //MyBitsReader reader(MyByteIterator(limit, positions->at(0), positions->at(1)));
                        MyBufferIterator buffers(limit, nalus[i].begin, nalus[i].end);
                        datas.insert(datas.end(), buffers, end);
                        sample.size += nalus[i].size + 4;
                //}
            }
            sample.data.swap(datas);
        }

    }
}
