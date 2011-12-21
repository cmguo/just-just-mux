// ParseH264Transfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/ParseH264Transfer.h"
#include "ppbox/mux/detail/BitsReader.h"

namespace ppbox
{
    namespace mux
    {

        void ParseH264Transfer::transfer(ppbox::demux::Sample & sample)
        {
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
        }

    }
}
