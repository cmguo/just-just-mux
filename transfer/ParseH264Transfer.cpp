// ParseH264Transfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/ParseH264Transfer.h"
#include "ppbox/mux/detail/BitsReader.h"

#include <ppbox/avformat/stream/BitsOStream.h>
#include <ppbox/avformat/stream/BitsIStream.h>
#include <ppbox/avformat/stream/BitsBuffer.h>
#include <ppbox/avformat/codec/avc/AvcConfig.h>
#include <ppbox/avformat/codec/avc/AvcCodec.h>
#include <ppbox/avformat/codec/avc/AvcType.h>
#include <ppbox/avformat/stream/FormatBuffer.h>
using namespace ppbox::avformat;

#include <util/archive/ArchiveBuffer.h>
#include <util/buffers/CycleBuffers.h>

// This transfer is for debug

namespace ppbox
{
    namespace mux
    {

        std::map<boost::uint32_t, SeqParameterSetRbsp> spss;
        std::map<boost::uint32_t, PicParameterSetRbsp> ppss;

        void ParseH264Transfer::transfer(
            StreamInfo & info)        {            AvcConfig const & avc_config = 
                ((AvcCodec const *)info.codec)->config();
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

        void ParseH264Transfer::transfer(
            Sample & sample)
        {
            NaluList const & nalus = 
                *(NaluList const * )sample.context;
            util::buffers::BuffersLimit<ConstBuffers::const_iterator> limit(sample.data.begin(), sample.data.end());
            std::cout << "Frame: " << " dts: " << sample.dts << ",\t cts: " << sample.dts + sample.cts_delta << std::endl;
            for (boost::uint32_t i = 0; i < nalus.size(); ++i) {
                NaluHeader nalu_header(nalus[i].begin.dereference_byte());
                std::cout << "Nalu type: " 
                    << NaluHeader::nalu_type_str[nalu_header.nal_unit_type] 
                << std::endl;
                MyBufferIterator iter(limit, nalus[i].begin, nalus[i].end);
                if (nalu_header.nal_unit_type == NaluHeader::SEI) {
                    std::cout << "Sei: size = " << nalus[i].size << std::endl;
                    MyBuffers buffers(iter);
                    util::buffers::CycleBuffers<MyBuffers, boost::uint8_t> buf(buffers);
                    buf.commit(nalus[i].size);
                    BitsBuffer<boost::uint8_t> bits_buf(buf);
                    BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                    SeiRbsp sei(spss, ppss);
                    bits_reader >> sei;
                }
                if (nalu_header.nal_unit_type == NaluHeader::UNIDR
                    || nalu_header.nal_unit_type == NaluHeader::IDR) {
                        MyBuffers buffers(iter);
                        util::buffers::CycleBuffers<MyBuffers, boost::uint8_t> buf(buffers);
                        buf.commit(nalus[i].size);
                        BitsBuffer<boost::uint8_t> bits_buf(buf);
                        BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                        SliceLayerWithoutPartitioningRbsp slice(ppss);
                        bits_reader >> slice;
                        std::cout << "Slice type: " 
                            << SliceHeader::slice_type_str[slice.slice_header.slice_type] 
                        << std::endl;
                }
            }        }

    } // namespace mux
} // namespace ppbox
