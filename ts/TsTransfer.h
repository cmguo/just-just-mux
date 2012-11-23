// TsTransfer.h

#ifndef _PPBOX_MUX_TS_TS_TRANSFER_H_
#define _PPBOX_MUX_TS_TS_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <framework/system/ScaleTransform.h>

namespace ppbox
{
    namespace mux
    {

        class TsTransfer
            : public Transfer
        {
        public:
            TsTransfer(
                boost::uint16_t pid);

            ~TsTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer_time(
                Sample & sample);

            virtual void transfer(
                Sample & sample);

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
            boost::uint16_t pid_;
            boost::uint8_t continuity_counter_;
            bool with_pcr_;
            boost::uint8_t time_adjust_;
            framework::system::ScaleTransform scale_;
            // buffer
            std::vector<boost::uint8_t> header_buffer_;
            std::deque<boost::asio::const_buffer> ts_buffers_;
            std::vector<size_t> off_segs_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_TS_TS_TRANSFER_H_
