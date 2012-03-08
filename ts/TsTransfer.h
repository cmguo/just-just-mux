// TsStreamTransfer.h

#ifndef   _PPBOX_MUX_TS_STREAM_TRANSFER_H_
#define   _PPBOX_MUX_TS_STREAM_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"
#include "ppbox/mux/ts/Mpeg2Ts.h"

#include <framework/system/ScaleTransform.h>

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
                boost::uint16_t stream_id);

            ~TsTransfer();

        private:
            virtual void transfer(
                ppbox::mux::MediaInfoEx & media);

            virtual void transfer(
                ppbox::demux::Sample & sample);

            virtual void on_seek(
                boost::uint32_t time);

        private:
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
            boost::uint16_t stream_id_;
            boost::uint8_t time_adjust_;
            framework::system::ScaleTransform scale_;
            bool with_pcr_;
            bool with_dts_;
            // buffer
            std::vector<boost::uint8_t> ts_headers_;
            std::deque<boost::asio::const_buffer> ts_buffers_;
            std::vector<size_t> off_segs_;
            char pes_heaher_buffer_[19];
        };
    }
}
#endif // _PPBOX_MUX_TS_STREAM_TRANSFER_H_
