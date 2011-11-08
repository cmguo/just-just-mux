// PackageJoinTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/elements.h"
#include "ppbox/mux/transfer/PackageJoinTransfer.h"
#include "ppbox/mux/detail/BitsReader.h"

#include <framework/system/BytesOrder.h>

namespace ppbox
{
    namespace mux
    {

        void PackageJoinTransfer::transfer(ppbox::demux::Sample & sample)
        {
            NaluList const * nalus = 
                (NaluList const * )sample.context;
            util::buffers::BuffersLimit<ConstBuffers::const_iterator> limit(sample.data.begin(), sample.data.end());
            if (nalus->size() == 0) {
                return;
            }
            Nalu const * nalu = NULL;
            for (boost::uint32_t i = 0; i < nalus->size(); ++i) {
                Nal_header nalu_header = *(Nal_header*)&nalus->at(i).begin.dereference_byte();
                if (nalu_header.nal_unit_type == NALUType::IDR 
                    || nalu_header.nal_unit_type == NALUType::UNIDR) {
                        nalu = &nalus->at(i);
                        break;
                }
            }
            if (nalu == NULL) {
                nalu = &nalus->front();
            }
            std::deque<boost::asio::const_buffer> datas;
            frame_data_size_ = nalu->size;
            frame_data_size_ = framework::system::BytesOrder::host_to_big_endian_long(frame_data_size_);
            datas.push_back(boost::asio::buffer((boost::uint8_t*)&frame_data_size_, 4));
            MyBufferIterator iter(limit, nalu->begin, nalu->end);
            MyBufferIterator end;
            datas.insert(datas.end(), iter, end);
            sample.size = nalu->size + 4;
            sample.data.swap(datas);
        }

    }
}