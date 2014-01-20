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
                if (codec->stream_type <= 0x80) {
                    pmt_sec.add_stream((boost::uint8_t)codec->stream_type);
                } else {
                    PmtStream stream(0x80);
                    std::vector<boost::uint8_t> descriptor(
                        (boost::uint8_t *)&codec->stream_type, 
                        (boost::uint8_t *)&codec->stream_type + 4);
                    stream.add_descriptor(0x05, descriptor);
                    pmt_sec.add_stream(stream);
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
