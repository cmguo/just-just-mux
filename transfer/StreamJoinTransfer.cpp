// StreamJoinTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/StreamJoinTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"
#include "ppbox/mux/AvcConfig.h"

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
                AvcConfig const * stream_config = (AvcConfig const *)video_info->config;;
                nalu_length_ = stream_config->nalu_lengthSize();
                // start code
                nalu_start_code_.clear();
                for(boost::uint32_t i = 0; i < nalu_length_-1; ++i) {
                    nalu_start_code_.push_back(0);
                }
                nalu_start_code_.push_back(1);
                // access unit delimiter
                access_unit_delimiter_.clear();
                for(boost::uint32_t i = 0; i < nalu_length_-1; ++i) {
                    access_unit_delimiter_.push_back(0);
                }
                access_unit_delimiter_.push_back(1);
                access_unit_delimiter_.push_back(9);
                access_unit_delimiter_.push_back(0xE0);
                // sps
                for (boost::uint32_t i = 0; i < stream_config->sequence_parameters().size(); i++) {
                    for(boost::uint32_t j = 0; j < nalu_length_-1; ++j) {
                        sps_pps_.push_back(0);
                    }
                    sps_pps_.push_back(1);
                    for(boost::uint32_t pos = 0; pos < stream_config->sequence_parameters()[i].size(); ++pos) {
                        sps_pps_.push_back(stream_config->sequence_parameters()[i].at(pos));
                    }
                }
                // pps
                for (boost::uint32_t i = 0; i < stream_config->picture_parameters().size(); i++) {
                    for(boost::uint32_t j = 0; j < nalu_length_-1; ++j) {
                        sps_pps_.push_back(0);
                    }
                    sps_pps_.push_back(1);
                    for(boost::uint32_t pos = 0; pos < stream_config->picture_parameters()[i].size(); ++pos) {
                        sps_pps_.push_back(stream_config->picture_parameters()[i].at(pos));
                    }
                }
            }

            std::deque<boost::asio::const_buffer> datas;
            datas.push_back(boost::asio::buffer(&access_unit_delimiter_.at(0), access_unit_delimiter_.size()));
            sample.size += access_unit_delimiter_.size();
            if (sample.flags & demux::Sample::sync) {
                datas.push_back(boost::asio::buffer(sps_pps_));
                sample.size += sps_pps_.size();
            }
            NaluList const * nalus = 
                (NaluList const * )sample.context;
            util::buffers::BuffersLimit<ConstBuffers::const_iterator> limit(sample.data.begin(), sample.data.end());
            MyBufferIterator end;
            for (boost::uint32_t i = 0; i < nalus->size(); ++i) {
                datas.push_back(boost::asio::buffer(nalu_start_code_));
                //MyBitsReader reader(MyByteIterator(limit, positions->at(0), positions->at(1)));
                MyBufferIterator buffers(limit, nalus->at(i).begin, nalus->at(i).end);
                datas.insert(datas.end(), buffers, end);
            }
            sample.data.swap(datas);
        }

    }
}
