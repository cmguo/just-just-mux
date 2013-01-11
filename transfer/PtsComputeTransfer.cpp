// PtsComputeTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/PtsComputeTransfer.h"

#include <ppbox/avformat/stream/BitsIStream.h>
#include <ppbox/avformat/stream/BitsOStream.h>
#include <ppbox/avformat/stream/FormatBuffer.h>
#include <ppbox/avformat/stream/BitsBuffer.h>
#include <ppbox/avformat/codec/avc/AvcSliceType.h>
#include <ppbox/avformat/codec/avc/AvcNaluBuffer.h>
using namespace ppbox::avformat;

#include <util/buffers/CycleBuffers.h>

namespace ppbox
{
    namespace mux
    {

        template <typename T>
        static bool parse(
            T & t, 
            Sample & s, 
            NaluBuffer const & n)
        {
            util::buffers::CycleBuffers<SampleBuffers::RangeBuffers, boost::uint8_t> buf(n.buffers());
            buf.commit(n.size);
            BitsBuffer<boost::uint8_t> bits_buf(buf);
            BitsIStream<boost::uint8_t> bits_reader(bits_buf);
            bits_reader >> t;
            return !!bits_reader;
        }

        PtsComputeTransfer::PtsComputeTransfer()
            : idr_dts_(0)
            , frame_scale_(0)
            , is_last_a_idr_(false)
        {
        }

        void PtsComputeTransfer::transfer(
            Sample & sample)
        {
            if (sample.cts_delta != boost::uint32_t(-1)) {
                return;
            }

            std::vector<NaluBuffer> & nalus = 
                *(std::vector<NaluBuffer> *)sample.context;

            if (sample.flags & Sample::sync) {
                idr_dts_ = sample.dts;
                is_last_a_idr_ = true;
            } else if (is_last_a_idr_) {
                frame_scale_ = boost::uint32_t(sample.dts - idr_dts_);
                is_last_a_idr_ = false;
            }
            for (size_t i = 0; i < nalus.size(); ++i) {
                NaluBuffer const & nalu = nalus[i];
                NaluHeader nalu_header(nalu.begin.dereference_byte());
                if (NaluHeader::SPS == nalu_header.nal_unit_type) {
                    SeqParameterSetRbsp sps;
                    parse(sps, sample, nalu);
                    spss_.insert(std::make_pair(sps.sps_seq_parameter_set_id, sps));
                } else if (NaluHeader::PPS == nalu_header.nal_unit_type) {
                    PicParameterSetRbsp pps(spss_);
                    parse(pps, sample, nalu);
                    ppss_.insert(std::make_pair(pps.pps_pic_parameter_set_id, pps));
                } else if (NaluHeader::UNIDR == nalu_header.nal_unit_type
                    || NaluHeader::IDR == nalu_header.nal_unit_type) {
                    SliceLayerWithoutPartitioningRbsp slice(ppss_);
                    parse(slice, sample, nalu);
                    sample.cts_delta = (boost::uint32_t)(idr_dts_ + frame_scale_ * slice.slice_header.pic_order_cnt_lsb / 2 - sample.dts);
                    // iphoneÂ¼ÖÆÊ¹ÓÃ
                    if (slice.slice_header.slice_type % 5 == 2) {
                        sample.flags |= Sample::sync;
                    }
                } else {
                    // skip
                }
            } // End for
        }

    } // namespace mux
} // namespace ppbox
