// TsMuxer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/TsMuxer.h"
#include "ppbox/mux/ts/PesTransfer.h"

#include <ppbox/avformat/ts/TsFormat.h>
using namespace ppbox::avformat;

namespace ppbox
{
    namespace mux
    {

        TsMuxer::TsMuxer()
        {
        }

        TsMuxer::~TsMuxer()
        {
        }

        void TsMuxer::add_stream(
            StreamInfo & info, 
            FilterPipe & pipe)
        {
            Transfer * transfer = NULL;
            PmtSection & pmt_sec = pmt_.sections.front();
            TsFormat ts;
            boost::system::error_code ec;
            CodecInfo const * codec = ts.codec_from_codec(info.type, info.sub_type, ec);
            if (codec) {
                TsContext const * ctx = (TsContext const *)codec->context;
                if (ctx) {
                    PmtStream stream(0);
                    if (ctx->regd_type) {
                        std::vector<boost::uint8_t> descriptor(
                            (boost::uint8_t *)&ctx->regd_type, 
                            (boost::uint8_t *)&ctx->regd_type + 4);
                        stream.add_descriptor(0x05, descriptor);
                    }
                    if (ctx->hdmv_type) {
                        stream.stream_type = ctx->hdmv_type;
                        if (pmt_sec.descriptor.empty()) {
                            boost::uint8_t format[4] = {'H', 'D', 'M', 'V'};
                            std::vector<boost::uint8_t> descriptor(format, format + 4);
                            pmt_sec.add_descriptor(0x05, descriptor);
                        }
                    } else if (ctx->misc_type) {
                        stream.stream_type = ctx->misc_type;
                    }
                    pmt_sec.add_stream(stream);
                } else {
                    pmt_sec.add_stream((boost::uint8_t)codec->stream_type);
                }
            }
            transfer = new PesTransfer(info.index);
            pipe.insert(transfer);
        }

        void TsMuxer::file_header(
            Sample & sample)
        {
            pmt_.complete();

            FormatBuffer buf(header_buffer_, sizeof(header_buffer_));
            TsOArchive oa(buf);
            oa << pat_;
            oa << pmt_;
            assert(buf.size() == TsPacket::PACKET_SIZE * 2);

            sample.data.push_back(buf.data());
        }

        void TsMuxer::stream_header(
            boost::uint32_t index, 
            Sample & sample)
        {
        }

    } // namespace mux
} // namespace ppbox
