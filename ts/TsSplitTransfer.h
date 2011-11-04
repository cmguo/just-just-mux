// TsSplitTransfer.h

#ifndef   _PPBOX_MUX_TS_SPLIT_TRANSFER_H_
#define   _PPBOX_MUX_TS_SPLIT_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/ts/Mpeg2Ts.h"
#include "ppbox/mux/detail/BitsReader.h"

namespace ppbox
{
    namespace mux
    {
        class TsSplitTransfer
            : public Transfer
        {
        public:
            TsSplitTransfer(boost::uint32_t size = 7)
                : max_ts_packet_(size)
            {
            }

            ~TsSplitTransfer()
            {
            }

            virtual void transfer(ppbox::demux::Sample & sample)
            {
                ts_packets_.clear();
                boost::int32_t frame_size = util::buffers::buffer_size(sample.data);
                MyBuffersLimit limit(sample.data.begin(), sample.data.end());
                MyBuffersPosition position(limit);
                MyBuffersPosition pos;
                while (frame_size > 0) {
                    pos = position;
                    position.increment_bytes(limit, AP4_MPEG2TS_PACKET_SIZE*max_ts_packet_);
                    frame_size -= AP4_MPEG2TS_PACKET_SIZE*max_ts_packet_;
                    push_buffers(MyBufferIterator(limit, pos, position), MyBufferIterator());
                }
                sample.context = (void*)&ts_packets_;
            }

            template <typename ConstBuffers>
            void push_buffers(
                ConstBuffers const & buffers1)
            {
                TsPacket ts_packet;
                ts_packet.buffers.insert(ts_packet.buffers.end(), buffers1.begin(), buffers1.end());
                ts_packets_.push_back(ts_packet);
            }

            template <typename ConstBuffersIterator>
            void push_buffers(
                ConstBuffersIterator const & beg, 
                ConstBuffersIterator const & end)
            {
                TsPacket ts_packet;
                ts_packet.buffers.insert(ts_packet.buffers.end(), beg, end);
                ts_packets_.push_back(ts_packet);
            }

        private:
            std::vector<TsPacket> ts_packets_;
            boost::uint32_t      max_ts_packet_;
        };
    }
}

#endif // _PPBOX_MUX_RTP_ES_TRANSFER_H_
