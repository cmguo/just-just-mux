// PtsComputeTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/PtsComputeTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"

namespace ppbox
{
    namespace mux
    {

        void PtsComputeTransfer::transfer(ppbox::demux::Sample & sample)
        {
            if (sample.cts_delta != boost::uint32_t(-1)) {
                return;
            }
            if (sample.is_sync) {
                idr_dts_ = sample.dts;
                is_last_a_idr_ = true;
            }
            if (sample.is_sync == false && is_last_a_idr_ == true) {
                frame_scale_ = boost::uint32_t(sample.dts - idr_dts_);
                is_last_a_idr_ = false;
            }
            NaluList const & nalus = 
                *(NaluList const * )sample.context;
            util::buffers::BuffersLimit<ConstBuffers::const_iterator> limit(sample.data.begin(), sample.data.end());
            for (boost::uint32_t i = 0; i < nalus.size(); ++i) {
                Nal_header nalu_header = *(Nal_header*)&nalus[i].begin.dereference_byte();
                if (NALUType::SPS == nalu_header.nal_unit_type) {
                    MyBitsReader reader(MyByteIterator(limit, nalus[i].begin, nalus[i].end), nalus[i].size);
                    nalu_parser_.parse_sps(reader);
                } else if (NALUType::PPS == nalu_header.nal_unit_type) {
                    MyBitsReader reader(MyByteIterator(limit, nalus[i].begin, nalus[i].end), nalus[i].size);
                    nalu_parser_.parse_pps(reader);
                } else if (NALUType::UNIDR == nalu_header.nal_unit_type
                    || NALUType::IDR == nalu_header.nal_unit_type) {
                        MyBitsReader reader(MyByteIterator(limit, nalus[i].begin, nalus[i].end), nalus[i].size);
                        nalu_parser_.parse_frame(reader);
                } else {
                    // skip
                }
            } // End for
            if (nalu_parser_.is_ready) {
                sample.cts_delta = idr_dts_ + frame_scale_*nalu_parser_.pic_order_cnt_lsb/2 - sample.dts;
            } else {
                sample.cts_delta = 0;
            }
        }

    }
}
