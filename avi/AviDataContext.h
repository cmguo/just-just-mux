// AviDataContext.h

#ifndef _PPBOX_MUX_AVI_AVI_DATA_CONTEXT_H_
#define _PPBOX_MUX_AVI_AVI_DATA_CONTEXT_H_

#include "ppbox/mux/MuxBase.h"

#include <ppbox/avformat/avi/lib/AviStream.h>

namespace ppbox
{
    namespace mux
    {

        class AviMuxer;

        class AviDataContext
        {
        public:
            AviDataContext();

        public:
            void open(
                boost::uint32_t block_size);

            void put_header(
                Sample & sample);

            void put_sample(
                ppbox::avformat::AviStream & stream, 
                Sample & sample);

            void pad_block(
                Sample & sample);

            void next_block(
                Sample & sample);

        private:
            friend class AviMuxer;

            boost::uint32_t block_size_;
            boost::uint64_t offset_;
            boost::uint64_t block_end_;
            std::vector<boost::uint8_t> movi_buf_;
            std::vector<boost::uint8_t> padding_;

        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_AVI_AVI_DATA_CONTEXT_H_
