// PackageSplitTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/transfer/PackageSplitTransfer.h"

#include <framework/system/BytesOrder.h>
#include <util/buffers/BufferCopy.h>

namespace ppbox
{
    namespace mux
    {

        void PackageSplitTransfer::transfer(ppbox::demux::Sample & sample)
        {
            nalus_.clear();
            MyBuffersLimit limit(sample.data.begin(), sample.data.end());
            MyBuffersPosition position(limit);
            while (!position.at_end()) {
                boost::uint32_t len = 0;
                util::buffers::buffer_copy(
                    boost::asio::buffer((char *)&len+4-nalu_length_, nalu_length_), 
                    util::buffers::Container<boost::asio::const_buffer, MyBufferIterator>(MyBufferIterator(limit, position)));
                len = framework::system::BytesOrder::net_to_host_long(len);
                position.increment_bytes(limit, nalu_length_);
                MyBuffersPosition pos = position;
                position.increment_bytes(limit, len);
                nalus_.push_back(Nalu(len, pos, position));
            }
            sample.context = (void *)&nalus_;
        }

    }
}
