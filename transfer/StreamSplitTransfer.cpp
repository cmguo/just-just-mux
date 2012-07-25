// StreamSplitTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/StreamSplitTransfer.h"

#include <ppbox/avformat/codec/AvcNalu.h>

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
                avformat::NaluHeader const nalu_header(finder.position().dereference_byte());
                if (nalu_header.nal_unit_type == avformat::NaluHeader::IDR ||
                    nalu_header.nal_unit_type == avformat::NaluHeader::UNIDR) {
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
