// Mp4Muxer.cpp

#include "just/mux/Common.h"
#include "just/mux/mp4/Mp4Muxer.h"
#include "just/mux/mp4/Mp4Transfer.h"

#include <just/avformat/mp4/box/Mp4BoxEnum.h>
using namespace just::avformat;

namespace just
{
    namespace mux
    {

        Mp4Muxer::Mp4Muxer(
            boost::asio::io_service & io_svc)
            : Muxer(io_svc)
            , block_size(1000 * 1024)
        {
            config().register_module("Mp4Muxer")
                << CONFIG_PARAM_RDONLY(block_size);
        }

        Mp4Muxer::~Mp4Muxer()
        {
        }

        void Mp4Muxer::do_open(
            MediaInfo & info)
        {
            context_.open(block_size);
            boost::system::error_code ec;
            file_.create(ec);
            file_.movie().time_scale(1000);
            if (media_info_.duration != (boost::uint64_t)-1)
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
            if (transfer)
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
            boost::system::error_code ec;
            file_.fixup(ec);
            context_.pad_block(sample);
            Mp4BoxContext ctx;
            Mp4BoxOArchive oa(tail_buffer_);
            oa.context(&ctx);
            oa << file_.movie().box();
            sample.size += tail_buffer_.size();
            sample.data.push_back(tail_buffer_.data());
        }

    } // namespace mux
} // namespace just
