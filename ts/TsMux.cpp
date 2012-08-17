// TsMux.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/TsMux.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/transfer/PackageSplitTransfer.h"
#include "ppbox/mux/transfer/StreamSplitTransfer.h"
#include "ppbox/mux/transfer/PtsComputeTransfer.h"
#include "ppbox/mux/transfer/StreamJoinTransfer.h"
#include "ppbox/mux/transfer/AdtsAudioTransfer.h"
#include "ppbox/mux/transfer/AudioMergeTransfer.h"
#include "ppbox/mux/ts/TsTransfer.h"
#include "ppbox/mux/ts/MpegTsType.h"

using namespace ppbox::demux;

#include <util/archive/ArchiveBuffer.h>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        static boost::uint32_t const CRC_Table[256] = {
                0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
                0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
                0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
                0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
                0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
                0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
                0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
                0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
                0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
                0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
                0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
                0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
                0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
                0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
                0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
                0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
                0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
                0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
                0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
                0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
                0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
                0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
                0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
                0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
                0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
                0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
                0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
                0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
                0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
                0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
                0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
                0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
                0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
                0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
                0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
                0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
                0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
                0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
                0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
                0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
                0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
                0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
                0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
        };

        static boost::uint32_t ComputeCRC(boost::uint8_t const * data, boost::uint32_t data_size)
        {
            boost::uint32_t crc = 0xFFFFFFFF;
            for (boost::uint32_t i=0; i<data_size; i++) {
                crc = (crc << 8) ^ CRC_Table[((crc >> 24) ^ *data++) & 0xFF];
            }
            return crc;
        }

        TsMux::TsMux()
            : pat_(new Stream(0))
            , pmt_(new Stream(AP4_MPEG2_TS_DEFAULT_PID_PMT))
            , has_audio_(false)
            , has_video_(false)
            , audio_pid_(AP4_MPEG2_TS_DEFAULT_PID_AUDIO)
            , video_pid_(AP4_MPEG2_TS_DEFAULT_PID_VIDEO)
            , audio_stream_id_(AP4_MPEG2_TS_DEFAULT_STREAM_ID_AUDIO)
            , video_stream_id_(AP4_MPEG2_TS_DEFAULT_STREAM_ID_VIDEO)
            , audio_stream_type_(AP4_MPEG2_STREAM_TYPE_ISO_IEC_13818_7)
            , video_stream_type_(AP4_MPEG2_STREAM_TYPE_AVC)
        {
        }

        TsMux::~TsMux()
        {
            if (pat_) {
                delete pat_;
                pat_ = NULL;
            }

            if (pmt_) {
                delete pmt_;
                pmt_ = NULL;
            }
        }

        void TsMux::add_stream(
            MediaInfoEx & mediainfo)
        {
            Transfer * transfer = NULL;
            if (mediainfo.type == ppbox::demux::MEDIA_TYPE_VIDE) {
                if (mediainfo.format_type == MediaInfo::video_avc_packet) {
                    transfer = new PackageSplitTransfer();
                    mediainfo.transfers.push_back(transfer);
                    transfer = new StreamJoinTransfer();
                    mediainfo.transfers.push_back(transfer);
                } else if (mediainfo.format_type == MediaInfo::video_avc_byte_stream) {
                    transfer = new StreamSplitTransfer();
                    mediainfo.transfers.push_back(transfer);
                    transfer = new PtsComputeTransfer();
                    mediainfo.transfers.push_back(transfer);
                }
                transfer = new TsTransfer(video_pid_, video_stream_id_);
                mediainfo.transfers.push_back(transfer);
                has_video_ = true;
            } else if (mediainfo.type == ppbox::demux::MEDIA_TYPE_AUDI) {
                has_audio_ = true;
                if (mediainfo.sub_type == ppbox::demux::AUDIO_TYPE_MP4A) {
                    transfer = new AdtsAudioTransfer();
                    mediainfo.transfers.push_back(transfer);
                } else if (mediainfo.sub_type == ppbox::demux::AUDIO_TYPE_MP1A) {
                    audio_stream_type_ = AP4_MPEG2_STREAM_TYPE_ISO_IEC_13818_3;
                }
                transfer = new TsTransfer(audio_pid_, audio_stream_id_);
                mediainfo.transfers.push_back(transfer);
            }
        }

        void TsMux::file_header(
            ppbox::demux::Sample & tag)
        {
            WritePAT(header_);
            WritePMT(header_ + 188);
            tag.data.clear();
            tag.time = 0;
            tag.ustime = 0;
            tag.dts = 0;
            tag.cts_delta = 0;
            tag.data.push_back(boost::asio::buffer(header_, 376));
        }

        void TsMux::stream_header(
            boost::uint32_t index, 
            ppbox::demux::Sample & tag)
        {
            tag.data.clear();
        }

        void TsMux::WritePAT(boost::uint8_t * ptr)
        {
            boost::uint32_t payload_size = AP4_MPEG2TS_PACKET_PAYLOAD_SIZE;
            std::vector<boost::uint8_t> ts_header;
            ts_header.resize(payload_size);
            boost::uint32_t head_size = payload_size;
            pat_->WritePacketHeader(true, payload_size, head_size, false, 0, &ts_header.at(0));
            memcpy(ptr, &ts_header.at(0), head_size);
            PSI_table table_header;
            table_header.pointer = 0;
            table_header.table_id = 0;
            table_header.section_syntax_indicator = 1;
            table_header.undef = 0;
            table_header.reserved = 3;
            table_header.section_length = 13;
            table_header.transport_stream_id = 0;
            table_header.reserved1 = 3;
            table_header.version_number = 0;
            table_header.current_next_indicator = 1;
            table_header.section_number = 0;
            table_header.last_section_number = 0;
            PAT_section pat_section;
            pat_section.program_number = 1;
            pat_section.reserved = 7;
            pat_section.Pid = pmt_->GetPID();

            util::archive::ArchiveBuffer<char> buf((char*)ptr +head_size, AP4_MPEG2TS_PACKET_SIZE - head_size);
            TsOArchive ts_archive(buf);
            ts_archive << table_header;
            ts_archive << pat_section;
            boost::uint32_t crc = ComputeCRC(ptr+1+head_size, 17-1-4);
            ts_archive & crc;
            std::vector<boost::uint8_t> suffer;
            suffer.resize(AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-17);
            suffer.assign(AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-17, 255);
            util::serialization::serialize_collection(ts_archive, suffer, AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-17);
        }

        void TsMux::WritePMT(boost::uint8_t * ptr)
        {
            // check that we have at least one media stream
            if (!has_audio_ && !has_video_) {
                return;
            }

            boost::uint32_t payload_size = AP4_MPEG2TS_PACKET_PAYLOAD_SIZE;
            std::vector<boost::uint8_t> ts_header;
            ts_header.resize(payload_size);
            boost::uint32_t head_size = payload_size;
            pmt_->WritePacketHeader(true, payload_size, head_size, false, 0, &ts_header.at(0));
            memcpy(ptr, &ts_header.at(0), head_size);

            AP4_BitWriter writer(1024);
            unsigned int section_length = 13;
            unsigned int pcr_pid = 0;
            if (has_audio_) {
                section_length += 5;
                pcr_pid = audio_pid_;
            }
            if (has_video_) {
                section_length += 5;
                pcr_pid = video_pid_;
            }
            PSI_table table_header;
            table_header.pointer = 0;
            table_header.table_id = 2;
            table_header.section_syntax_indicator = 1;
            table_header.undef = 0;
            table_header.reserved = 3;
            table_header.section_length = section_length;
            table_header.transport_stream_id = 1;
            table_header.reserved1 = 3;
            table_header.version_number = 0;
            table_header.current_next_indicator = 1;
            table_header.section_number = 0;
            table_header.last_section_number = 0;
            PMT_section pmt_section;
            pmt_section.reserved = 7;
            pmt_section.Pcr_Pid = pcr_pid;
            pmt_section.reserved1 = 0xF;
            pmt_section.program_info_length = 0;

            util::archive::ArchiveBuffer<char> buf((char*)ptr+head_size, AP4_MPEG2TS_PACKET_SIZE - head_size);
            TsOArchive ts_archive(buf);
            ts_archive << table_header;
            ts_archive << pmt_section;

            if (has_video_) {
                StreamInfo stream_info;
                stream_info.stream_type = video_stream_type_;
                stream_info.reserved = 7;
                stream_info.elementary_PID = video_pid_;
                stream_info.reserved1 = 0xF;
                stream_info.ES_info_length = 0;
                ts_archive << stream_info;
            }
            if (has_audio_) {
                StreamInfo stream_info;
                stream_info.stream_type = audio_stream_type_;
                stream_info.reserved = 7;
                stream_info.elementary_PID = audio_pid_;
                stream_info.reserved1 = 0xF;
                stream_info.ES_info_length = 0;
                ts_archive << stream_info;
            }

            boost::uint32_t crc = ComputeCRC(ptr+1+head_size, section_length-1);
            ts_archive & crc;
            std::vector<boost::uint8_t> suffer;
            suffer.resize(AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-(section_length+4));
            suffer.assign(AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-(section_length+4), 255);
            util::serialization::serialize_collection(
                ts_archive, 
                suffer, 
                AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-(section_length+4));
        }

    } // namespace mux
} // namespace ppbox
