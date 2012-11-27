// ParseH264Transfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/ParseH264Transfer.h"
#include "ppbox/mux/detail/BitsReader.h"

#include <ppbox/avformat/BitsOStream.h>
#include <ppbox/avformat/BitsIStream.h>
#include <ppbox/avformat/BitsBuffer.h>
#include <ppbox/avformat/codec/avc/AvcConfig.h>
#include <ppbox/avformat/codec/avc/AvcCodec.h>
#include <ppbox/avformat/codec/avc/AvcType.h>
#include <util/archive/ArchiveBuffer.h>
#include <util/buffers/CycleBuffers.h>

// This transfer is for debug

namespace ppbox
{
    namespace mux
    {

        std::map<boost::uint32_t, ppbox::avformat::SeqParameterSetRbsp> spss;
        std::map<boost::uint32_t, ppbox::avformat::PicParameterSetRbsp> ppss;

        void ParseH264Transfer::transfer(
            StreamInfo & info)        {            ppbox::avformat::AvcConfig const & avc_config = 
                ((ppbox::avformat::AvcCodec const *)info.codec)->config();
            for (boost::uint32_t i = 0; i < avc_config.sequenceParameterSetNALUnit.size(); i++) {
                std::vector<boost::uint8_t> sps_vec = avc_config.sequenceParameterSetNALUnit[i];
                util::archive::ArchiveBuffer<boost::uint8_t> buf((boost::uint8_t *)&sps_vec[0], sps_vec.size(), sps_vec.size());
                ppbox::avformat::BitsBuffer<boost::uint8_t> bits_buf(buf);
                ppbox::avformat::BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                ppbox::avformat::SeqParameterSetRbsp sps;
                bits_reader >> sps;
                //if (!sps.vui_parameters.fixed_frame_rate_flag) {
                //    sps.vui_parameters.num_units_in_tick = 1;
                //    sps.vui_parameters.time_scale = 50;
                //    sps.vui_parameters.fixed_frame_rate_flag = 1;
                //    sps_vec.resize(sps_vec.size() + 4);
                //    util::archive::ArchiveBuffer<boost::uint8_t> buf((boost::uint8_t *)&sps_vec[0], sps_vec.size());
                //    ppbox::avformat::BitsBuffer<boost::uint8_t> bits_buf(buf);
                //    ppbox::avformat::BitsOStream<boost::uint8_t> bits_writer(bits_buf);
                //    bits_writer << sps;
                //    sps_vec.resize(buf.size());
                //    
                //}
                spss.insert(std::make_pair(sps.sps_seq_parameter_set_id, sps));
            }
            // pps
            for (boost::uint32_t i = 0; i < avc_config.pictureParameterSetNALUnit.size(); i++) {
                std::vector<boost::uint8_t> const & pps_vec = avc_config.pictureParameterSetNALUnit[i];
                util::archive::ArchiveBuffer<boost::uint8_t> buf((boost::uint8_t *)&pps_vec[0], pps_vec.size(), pps_vec.size());
                ppbox::avformat::BitsBuffer<boost::uint8_t> bits_buf(buf);
                ppbox::avformat::BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                ppbox::avformat::PicParameterSetRbsp pps(spss);
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
                avformat::NaluHeader nalu_header(nalus[i].begin.dereference_byte());
                std::cout << "Nalu type: " 
                    << ppbox::avformat::NaluHeader::nalu_type_str[nalu_header.nal_unit_type] 
                    << std::endl;
                MyBufferIterator iter(limit, nalus[i].begin, nalus[i].end);
                if (nalu_header.nal_unit_type == avformat::NaluHeader::SEI) {
                    std::cout << "Sei: size = " << nalus[i].size << std::endl;
                    MyBuffers buffers(iter);
                    util::buffers::CycleBuffers<MyBuffers, boost::uint8_t> buf(buffers);
                    buf.commit(nalus[i].size);
                    ppbox::avformat::BitsBuffer<boost::uint8_t> bits_buf(buf);
                    ppbox::avformat::BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                    ppbox::avformat::SeiRbsp sei(spss, ppss);
                    bits_reader >> sei;
                }
                if (nalu_header.nal_unit_type == avformat::NaluHeader::UNIDR
                    || nalu_header.nal_unit_type == avformat::NaluHeader::IDR) {
                        MyBuffers buffers(iter);
                        util::buffers::CycleBuffers<MyBuffers, boost::uint8_t> buf(buffers);
                        buf.commit(nalus[i].size);
                        ppbox::avformat::BitsBuffer<boost::uint8_t> bits_buf(buf);
                        ppbox::avformat::BitsIStream<boost::uint8_t> bits_reader(bits_buf);
                        ppbox::avformat::SliceLayerWithoutPartitioningRbsp slice(ppss);
                        bits_reader >> slice;
                        std::cout << "Slice type: " 
                            << ppbox::avformat::SliceHeader::slice_type_str[slice.slice_header.slice_type] 
                            << std::endl;
                }
            }
        }

    } // namespace mux
} // namespace ppbox
