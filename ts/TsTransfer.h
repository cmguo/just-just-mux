// TsStreamTransfer.h

#ifndef   _PPBOX_MUX_TS_STREAM_TRANSFER_H_
#define   _PPBOX_MUX_TS_STREAM_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/ts/Mpeg2Ts.h"

namespace ppbox
{
    namespace mux
    {

        class TsTransfer
            : public Stream
        {
        public:
            TsTransfer(
                boost::uint16_t pid,
                boost::uint16_t stream_id,
                boost::uint8_t stream_type,
                boost::uint32_t timescale)
                : Stream(pid)
                , pid_(pid)
                , stream_id_(stream_id)
                , stream_type_(stream_type)
                , timescale_(timescale)
            {
            }

            ~TsTransfer()
            {
            }

            virtual void transfer(ppbox::demux::Sample & sample);

        private:
            virtual void WritePES(
                ppbox::demux::Sample & sample,
                boost::uint64_t dts, 
                bool with_dts, 
                boost::uint64_t pts, 
                bool with_pcr);

            template <typename ConstBuffers>
            void push_buffers(
                ConstBuffers const & buffers1)
            {
                ts_buffers_.insert(ts_buffers_.end(), buffers1.begin(), buffers1.end());
            }

            template <typename ConstBuffersIterator>
            void push_buffers(
                ConstBuffersIterator const & beg, 
                ConstBuffersIterator const & end)
            {
                ts_buffers_.insert(ts_buffers_.end(), beg, end);
            }

        private:
            boost::uint16_t pid_;
            boost::uint16_t stream_id_;
            boost::uint8_t  stream_type_;
            boost::uint32_t timescale_;

            // buffer
            std::vector<boost::uint8_t> ts_headers_;
            std::deque<boost::asio::const_buffer> ts_buffers_;
            std::vector<size_t> off_segs_;
            char pes_heaher_buffer_[19];
            boost::uint32_t pes_header_size_;
        };
    }
}
#endif // _PPBOX_MUX_TS_STREAM_TRANSFER_H_
