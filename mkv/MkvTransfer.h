//MkvTransfer.h
#ifndef _PPBOX_MUX_MKV_MKV_TRANSFER_H_
#define _PPBOX_MUX_MKV_MKV_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"

#include <ppbox/avformat/mkv/MkvObjectType.h>

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
            ~MkvTransfer();

            virtual void transfer(
                ppbox::demux::MediaInfo & mediainfo);

            virtual void transfer(
                ppbox::demux::Sample & sample);

            void on_seek();

        private:
            boost::asio::streambuf block_head_buf_;
            boost::uint64_t add_cluster_flag_;
            boost::uint64_t time_code_;
        };

    }//mux
}//ppbox


#endif


