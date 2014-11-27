// AviDataContext.h

#ifndef _JUST_MUX_AVI_AVI_DATA_CONTEXT_H_
#define _JUST_MUX_AVI_AVI_DATA_CONTEXT_H_

#include "just/mux/MuxBase.h"

#include <just/avformat/avi/lib/AviStream.h>

namespace just
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
                just::avformat::AviStream & stream, 
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
} // namespace just

#endif // _JUST_MUX_AVI_AVI_DATA_CONTEXT_H_
