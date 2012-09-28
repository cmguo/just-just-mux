// PtsComputeTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/PtsComputeTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"

#include <ppbox/avformat/BitsIStream.h>
#include <ppbox/avformat/BitsOStream.h>
#include <ppbox/avformat/BitsBuffer.h>
#include <ppbox/avformat/codec/AvcConfig.h>
#include <ppbox/avformat/codec/AvcSliceType.h>

#include <util/archive/ArchiveBuffer.h>
#include <util/buffers/CycleBuffers.h>

namespace ppbox
{
    namespace mux
    {

        template <typename T>
        static bool parse(
            T & t, 
            ppbox::demux::Sample & s, 
            Nalu const & n)
        {
            util::buffers::BuffersLimit<ConstBuffers::const_iterator> limit(s.data.begin(), s.data.end());
            MyBufferIterator iter(limit, n.begin, n.end);
            MyBuffers buffers(iter);
            util::buffers::CycleBuffers<MyBuffers, boost::uint8_t> buf(buffers);
            buf.commit(n.size);
            ppbox::avformat::BitsBuffer<boost::uint8_t> bits_buf(buf);
            ppbox::avformat::BitsIStream<boost::uint8_t> bits_reader(bits_buf);
            bits_reader >> t;
            return !!bits_reader;
        }

        void PtsComputeTransfer::transfer(
            ppbox::demux::Sample & sample)
        {
            if (sample.cts_delta != boost::uint32_t(-1)) {
                return;
            }
            if (sample.flags & demux::Sample::sync) {
                idr_dts_ = sample.dts;
                is_last_a_idr_ = true;
            } else if (is_last_a_idr_) {
                frame_scale_ = boost::uint32_t(sample.dts - idr_dts_);
                is_last_a_idr_ = false;
            }
            NaluList const & nalus = 
                *(NaluList const * )sample.context;
            util::buffers::BuffersLimit<ConstBuffers::const_iterator> limit(sample.data.begin(), sample.data.end());
            for (size_t i = 0; i < nalus.size(); ++i) {
                avformat::NaluHeader nalu_header(nalus[i].begin.dereference_byte());
                if (avformat::NaluHeader::SPS == nalu_header.nal_unit_type) {
                    ppbox::avformat::SeqParameterSetRbsp sps;
                    parse(sps, sample, nalus[i]);
                    spss_.insert(std::make_pair(sps.sps_seq_parameter_set_id, sps));
                } else if (avformat::NaluHeader::PPS == nalu_header.nal_unit_type) {
                    ppbox::avformat::PicParameterSetRbsp pps(spss_);
                    parse(pps, sample, nalus[i]);
                    ppss_.insert(std::make_pair(pps.pps_pic_parameter_set_id, pps));
                } else if (avformat::NaluHeader::UNIDR == nalu_header.nal_unit_type
                    || avformat::NaluHeader::IDR == nalu_header.nal_unit_type) {
                    ppbox::avformat::SliceLayerWithoutPartitioningRbsp slice(ppss_);
                    parse(slice, sample, nalus[i]);
                    sample.cts_delta = (boost::uint32_t)(idr_dts_ + frame_scale_ * slice.slice_header.pic_order_cnt_lsb / 2 - sample.dts);
                    // iphoneÂ¼ÖÆÊ¹ÓÃ
                    if (slice.slice_header.slice_type % 5 == 2) {
                        sample.flags |= ppbox::demux::Sample::sync;
                    }
                } else {
                    // skip
                }
            } // End for
        }

    }
}
