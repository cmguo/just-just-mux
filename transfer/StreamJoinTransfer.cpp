// StreamJoinTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/StreamJoinTransfer.h"

#include <ppbox/avformat/codec/avc/AvcNalu.h>
#include <ppbox/avformat/codec/avc/AvcCodec.h>
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        StreamJoinTransfer::StreamJoinTransfer()
            : sample_description_index_(boost::uint32_t(-1))
        {
            // start code
        }

        StreamJoinTransfer::~StreamJoinTransfer()
        {
        }

        void StreamJoinTransfer::transfer(
            StreamInfo & info)
        {
            AvcConfigHelper const & avc_config = 
                ((AvcCodec const *)info.codec.get())->config_helper();
            // access unit delimiter
            boost::uint8_t nalu_start_code[] = {0, 0, 0, 1};
            access_unit_delimiter_.assign(nalu_start_code, nalu_start_code + 4);
            access_unit_delimiter_.push_back(9);
            access_unit_delimiter_.push_back(0xF0);
            // sps
            avc_config.to_es_data(sps_pps_);

            sample_description_index_ = boost::uint32_t(-1);
        }

        void StreamJoinTransfer::transfer(
            Sample & sample)
        {
            std::vector<NaluBuffer> & nalus = 
                *(std::vector<NaluBuffer> *)sample.context;

            //packet -> es
            if (0 != sample_description_index_) {
                sample_description_index_ = 0;
                bool need_aud = true;
                bool need_sps_pps = true;
                for (boost::uint32_t i = 0; i < nalus.size(); ++i) {
                    NaluBuffer const & nalu = nalus[i];
                    NaluHeader nalu_header(nalu.begin.dereference_byte());
                    if (nalu_header.nal_unit_type == NaluHeader::AccessUnitDelimiter) {
                        need_aud = false;
                    }
                    if (nalu_header.nal_unit_type == NaluHeader::SPS) {
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

            sample.size = 0;
            helper_.nalus(nalus);
            helper_.to_stream(sample.size, sample.data);

            if (sample.flags & demux::Sample::sync) { 
                sample.data.push_front(boost::asio::buffer(sps_pps_));
                sample.size += sps_pps_.size();
            }
            sample.data.push_front(boost::asio::buffer(access_unit_delimiter_));
            sample.size += access_unit_delimiter_.size();

            sample.context = NULL;
        }

    } // namespace mux
} // namespace ppbox
