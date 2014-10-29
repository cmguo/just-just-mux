// AviMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/avi/AviMuxer.h"
#include "ppbox/mux/avi/AviTransfer.h"

#include <ppbox/avformat/avi/box/AviBoxEnum.h>
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        AviMuxer::AviMuxer(
            boost::asio::io_service & io_svc)
            : Muxer(io_svc)
            , block_size(1000 * 1024)
        {
            config().register_module("AviMuxer")
                << CONFIG_PARAM_RDONLY(block_size);
        }

        AviMuxer::~AviMuxer()
        {
        }

        void AviMuxer::do_open(
            MediaInfo & info)
        {
            context_.open(block_size);
            boost::system::error_code ec;
            file_.create(ec);
        }

        void AviMuxer::add_stream(
            StreamInfo & info, 
            FilterPipe & pipe)
        {
            AviTransfer * transfer = NULL;
            if (info.type == StreamType::VIDE) {
                transfer = new AviTransfer(file_.header_list()->create_stream(AviStreamType::vids), &context_);
            } else {
                transfer = new AviTransfer(file_.header_list()->create_stream(AviStreamType::auds), &context_);
            }
            if (transfer)
                pipe.insert(transfer);
        }

        void AviMuxer::file_header(
            Sample & sample)
        {
            AviBoxOArchive oa(head_buffer_);
            boost::system::error_code ec;
            file_.fixup(ec);
            file_.save(oa);
            sample.size = head_buffer_.size();
            sample.data.push_back(head_buffer_.data());
            context_.put_header(sample);
        }

        void AviMuxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
        }

        void AviMuxer::file_tail(
            Sample & sample)
        {
            context_.pad_block(sample);
            AviBoxContext ctx;
            AviBoxOArchive oa(tail_buffer_);
            oa.context(&ctx);
            oa << file_.index()->box();
            sample.size += tail_buffer_.size();
            sample.data.push_back(tail_buffer_.data());
        }

    } // namespace mux
} // namespace ppbox
