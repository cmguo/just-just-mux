// StreamSplitTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/elements.h"
#include "ppbox/mux/transfer/StreamSplitTransfer.h"

namespace ppbox
{
    namespace mux
    {

        void StreamSplitTransfer::transfer(ppbox::demux::Sample & sample)
        {
            nalus_.clear();
            MyFindIterator2 finder(sample.data, boost::asio::buffer("\0\0\0\1", 4));
            MyFindIterator2 end;
            while (finder != end) {
                finder.skip_bytes(4);
                MyBuffersPosition pos = finder.position();
                Nal_header const nalu_header = *(Nal_header*)&finder.position().dereference_byte();
                if (nalu_header.nal_unit_type == NALUType::IDR ||
                    nalu_header.nal_unit_type == NALUType::UNIDR) {
                        nalus_.push_back(Nalu(
                            sample.size-pos.skipped_bytes(),
                            pos,
                            finder.end_position()));
                        break;
                }
                finder++;
                nalus_.push_back(Nalu(finder.position().skipped_bytes()-pos.skipped_bytes(), pos, finder.position()));
            }
            sample.context = (void*)&nalus_;
        }

    }
}
