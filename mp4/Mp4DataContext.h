// Mp4DataContext.h

#ifndef _PPBOX_MUX_MP4_MP4_DATA_CONTEXT_H_
#define _PPBOX_MUX_MP4_MP4_DATA_CONTEXT_H_

#include "ppbox/mux/MuxBase.h"

#include <ppbox/avformat/mp4/lib/Mp4SampleTable.h>

namespace ppbox
{
    namespace mux
    {

        class Mp4DataContext
        {
        public:
            Mp4DataContext(
                boost::uint32_t block_size);

        public:
            void put_header(
                Sample & sample);

            void put_sample(
                ppbox::avformat::Mp4SampleTable & table_, 
                Sample & sample);

            void pad_block(
                Sample & sample);

            void next_block(
                Sample & sample);

        private:
            boost::uint32_t block_size_;
            boost::uint64_t offset_;
            boost::uint64_t block_end_;
            std::vector<boost::uint8_t> mdat_;
            std::vector<boost::uint8_t> padding_;

        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MP4_MP4_DATA_CONTEXT_H_
