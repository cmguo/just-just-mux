// RtpEsAudioTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/rtp/RtpMpeg4GenericTransfer.h"

#include <framework/string/Base16.h>

namespace ppbox
{
    namespace mux
    {

        RtpMpeg4GenericTransfer::RtpMpeg4GenericTransfer()
            : RtpTransfer("RtpMpeg4Generic", 97)
            , index_(0)
        {
            au_header_section_[0] = 0;
            au_header_section_[1] = 16;
        }

        RtpMpeg4GenericTransfer::~RtpMpeg4GenericTransfer()
        {
        }

        void RtpMpeg4GenericTransfer::transfer(
            StreamInfo & info)
        {
            using namespace framework::string;

            RtpTransfer::transfer(info);

            std::string map_id_str = format(rtp_head_.mpt);
            rtp_info_.sdp = "m=audio 0 RTP/AVP " + map_id_str + "\r\n";
            rtp_info_.sdp += "a=rtpmap:" + map_id_str + " mpeg4-generic/" 
                + format(scale_.scale_out())
                + "/" + format(info.audio_format.channel_count) + "\r\n";
            rtp_info_.sdp += "a=fmtp:" + map_id_str 
                + " streamType=5"
                + ";profile-level-id=41"
                //+ ";objecttype=64"
                + ";mode=AAC-hbr"
                + ";sizeLength=13"
                + ";indexLength=3"
                + ";indexDeltaLength=3"
                + ";config=" + Base16::encode(std::string((char const *)&info.format_data.at(0), info.format_data.size()))
                + "\r\n";
            rtp_info_.sdp += "a=control:track" + format(info.index) + "\r\n";

            rtp_info_.stream_index = info.index;
        }

        void RtpMpeg4GenericTransfer::transfer(
            Sample & sample)
        {
            RtpTransfer::transfer(sample); // call TimeScaleTransfer::transfer

            au_header_section_[2] =  (boost::uint8_t)(sample.size >> 5);
            au_header_section_[3] = (boost::uint8_t)((sample.size << 3) /*| (index_++ & 0x07)*/);

            RtpTransfer::begin(sample);

            begin_packet(true, sample.dts, 4 + sample.size);
            push_buffers(boost::asio::buffer(au_header_section_, 4));
            push_buffers(sample.data);
            finish_packet();

            RtpTransfer::finish(sample);
        }

    } // namespace mux
} // namespace ppbox
