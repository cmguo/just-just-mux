// H264DebugTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/H264DebugTransfer.h"

#include <ppbox/avcodec/avc/AvcConfig.h>
#include <ppbox/avcodec/avc/AvcConfigHelper.h>
#include <ppbox/avcodec/avc/AvcType.h>
#include <ppbox/avcodec/avc/AvcNaluBuffer.h>
using namespace ppbox::avcodec;

#include <ppbox/avbase/stream/BitsOStream.h>
#include <ppbox/avbase/stream/BitsIStream.h>
#include <ppbox/avbase/stream/BitsBuffer.h>
#include <ppbox/avbase/stream/FormatBuffer.h>
using namespace ppbox::avbase;

#include <util/archive/ArchiveBuffer.h>
#include <util/buffers/CycleBuffers.h>

// This transfer is for debug

namespace ppbox
{
    namespace mux
    {

        std::map<boost::uint32_t, SeqParameterSetRbsp> spss;
        std::map<boost::uint32_t, PicParameterSetRbsp> ppss;

        void H264DebugTransfer::transfer(
            StreamInfo & info)
        {
            AvcConfig const & avc_config = 
                ((AvcConfigHelper const *)info.context)->data();
            for (boost::uint32_t i = 0; i < avc_config.sequenceParameterSetNALUnit.size(); i++) {
                std::vector<boost::uint8_t> sps_vec = avc_config.sequenceParameterSetNALUnit[i];
                FormatBuffer buf((boost::uint8_t *)&sps_vec[0], sps_vec.size(), sps_vec.size());
                BitsBuffer<boost::uint8_t> bits_buf(buf);
                BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                SeqParameterSetRbsp sps;
                bits_reader >> sps;
                //if (!sps.vui_parameters.fixed_frame_rate_flag) {
                //    sps.vui_parameters.num_units_in_tick = 1;
                //    sps.vui_parameters.time_scale = 50;
                //    sps.vui_parameters.fixed_frame_rate_flag = 1;
                //    sps_vec.resize(sps_vec.size() + 4);
                //    FormatBuffer buf((boost::uint8_t *)&sps_vec[0], sps_vec.size());
                //    BitsBuffer<boost::uint8_t> bits_buf(buf);
                //    BitsOStream<boost::uint8_t> bits_writer(bits_buf);
                //    bits_writer << sps;
                //    sps_vec.resize(buf.size());
                //    
                //}
                spss.insert(std::make_pair(sps.sps_seq_parameter_set_id, sps));
            }
            // pps
            for (boost::uint32_t i = 0; i < avc_config.pictureParameterSetNALUnit.size(); i++) {
                std::vector<boost::uint8_t> const & pps_vec = avc_config.pictureParameterSetNALUnit[i];
                FormatBuffer buf((boost::uint8_t *)&pps_vec[0], pps_vec.size(), pps_vec.size());
                BitsBuffer<boost::uint8_t> bits_buf(buf);
                BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                PicParameterSetRbsp pps(spss);
                bits_reader >> pps;
                ppss.insert(std::make_pair(pps.pps_pic_parameter_set_id, pps));
            }
        }

        void H264DebugTransfer::transfer(
            Sample & sample)
        {
            std::vector<NaluBuffer> & nalus = 
                *(std::vector<NaluBuffer> *)sample.context;

            std::cout << "Frame: " << " dts: " << sample.dts << ",\t cts: " << sample.dts + sample.cts_delta << std::endl;
            for (boost::uint32_t i = 0; i < nalus.size(); ++i) {
                NaluBuffer const & nalu = nalus[i];
                NaluHeader nalu_header(nalu.begin.dereference_byte());
                std::cout << "Nalu type: " << NaluHeader::nalu_type_str[nalu_header.nal_unit_type] << std::endl;

                if (nalu_header.nal_unit_type == NaluHeader::SEI) {
                    std::cout << "Sei: size = " << nalus[i].size << std::endl;
                    util::buffers::CycleBuffers<NaluBuffer::RangeBuffers, boost::uint8_t> buf(nalu.buffers());
                    buf.commit(nalus[i].size);
                    BitsBuffer<boost::uint8_t> bits_buf(buf);
                    BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                    SeiRbsp sei(spss, ppss);
                    bits_reader >> sei;
                }
                if (nalu_header.nal_unit_type == NaluHeader::UNIDR
                    || nalu_header.nal_unit_type == NaluHeader::IDR) {
                        util::buffers::CycleBuffers<NaluBuffer::RangeBuffers, boost::uint8_t> buf(nalu.buffers());
                        buf.commit(nalus[i].size);
                        BitsBuffer<boost::uint8_t> bits_buf(buf);
                        BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                        SliceLayerWithoutPartitioningRbsp slice(ppss);
                        bits_reader >> slice;
                        std::cout << "Slice type: " 
                            << SliceHeader::slice_type_str[slice.slice_header.slice_type] 
                        << std::endl;
                }
            }
        }

    } // namespace mux
} // namespace ppbox
