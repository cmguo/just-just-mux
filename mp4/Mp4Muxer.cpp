// Mp4Muxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/mp4/Mp4Muxer.h"
#include "ppbox/mux/mp4/Mp4Transfer.h"

#include <ppbox/avformat/mp4/box/Mp4BoxEnum.h>
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        Mp4Muxer::Mp4Muxer()
            : context_(100 * 1024)
        {
        }

        Mp4Muxer::~Mp4Muxer()
        {
        }

        void Mp4Muxer::do_open(
            MediaInfo & info)
        {
            boost::system::error_code ec;
            file_.create(ec);
            file_.movie().time_scale(1000);
            file_.movie().duration(media_info_.duration);
        }

        void Mp4Muxer::add_stream(
            StreamInfo & info, 
            FilterPipe & pipe)
        {
            Mp4Transfer * transfer = NULL;
            if (info.type == StreamType::VIDE) {
                transfer = new Mp4Transfer(file_.movie().create_track(Mp4HandlerType::vide), &context_);
            } else {
                transfer = new Mp4Transfer(file_.movie().create_track(Mp4HandlerType::soun), &context_);
            }
            pipe.insert(transfer);
        }

        void Mp4Muxer::file_header(
            Sample & sample)
        {
            Mp4BoxOArchive oa(head_buffer_);
            file_.save(oa);
            sample.size = head_buffer_.size();
            sample.data.push_back(head_buffer_.data());
            context_.put_header(sample);
        }

        void Mp4Muxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
        }

        void Mp4Muxer::file_tail(
            Sample & sample)
        {
            context_.pad_block(sample);
            Mp4BoxContext ctx;
            Mp4BoxOArchive oa(tail_buffer_);
            oa.context(&ctx);
            oa << file_.movie().box();
            sample.size += tail_buffer_.size();
            sample.data.push_back(tail_buffer_.data());
        }

    } // namespace mux
} // namespace ppbox
