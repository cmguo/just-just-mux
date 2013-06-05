// MkvTransfer.h

#ifndef _PPBOX_MUX_MKV_MKV_TRANSFER_H_
#define _PPBOX_MUX_MKV_MKV_TRANSFER_H_

#include "ppbox/mux/Transfer.h"

#include <boost/asio/streambuf.hpp>

namespace ppbox
{
    namespace mux
    {

        class MkvTransfer
            : public Transfer
        {
        public:
            MkvTransfer();

            virtual ~MkvTransfer();

        public:
            virtual void transfer(
                StreamInfo & info);

            virtual void transfer(
                Sample & sample);

            virtual void on_seek(
                boost::uint64_t time);

        public:
            void file_header(
                MediaInfo const & info, 
                size_t stream_obj_size, 
                boost::asio::streambuf & buf);

            void stream_header(
                StreamInfo const & info, 
                boost::asio::streambuf & buf);

        private:
            boost::uint8_t block_head_buf_[64];
            boost::uint64_t add_cluster_flag_;
            boost::uint64_t time_code_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MKV_MKV_TRANSFER_H_
