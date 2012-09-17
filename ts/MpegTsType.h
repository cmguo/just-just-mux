// MpegTsType.h

#ifndef   _PPBOX_MUX_MPEG_TS_DATA_TYPE_
#define   _PPBOX_MUX_MPEG_TS_DATA_TYPE_

#include <util/archive/BigEndianBinaryIArchive.h>
#include <util/archive/BigEndianBinaryOArchive.h>

namespace ppbox
{
    namespace mux
    {

#ifdef _DEBUG
#else
#endif // _DEBUG

        typedef util::archive::BigEndianBinaryIArchive<char>   TsIArchive;
        typedef util::archive::BigEndianBinaryOArchive<char>   TsOArchive;

        struct AdaptationField
        {
            boost::uint8_t adaptation_field_length;
            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint8_t discontinuity_indicator : 1;
                    boost::uint8_t random_access_indicator : 1;
                    boost::uint8_t elementary_stream_priority_indicator : 1;
                    boost::uint8_t PCR_flag : 1;
                    boost::uint8_t OPCR_flag : 1;
                    boost::uint8_t splicing_point_flag : 1;
                    boost::uint8_t transport_private_data_flag : 1;
                    boost::uint8_t adaptation_field_extension_flag : 1;
#else 
                    boost::uint8_t adaptation_field_extension_flag : 1;
                    boost::uint8_t transport_private_data_flag : 1;
                    boost::uint8_t splicing_point_flag : 1;
                    boost::uint8_t OPCR_flag : 1;
                    boost::uint8_t PCR_flag : 1;
                    boost::uint8_t elementary_stream_priority_indicator : 1;
                    boost::uint8_t random_access_indicator : 1;
                    boost::uint8_t discontinuity_indicator : 1;
#endif
                };
                boost::uint8_t flag;
            };

            // if (PCR_flag == 1) bit length (48bit)
            boost::uint32_t program_clock_reference_base; // 33
            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t program_clock_reference_base_last1bit : 1;
                    boost::uint16_t pcr_reserved : 6;
                    boost::uint16_t program_clock_reference_extension : 9;
#else 
                    boost::uint16_t program_clock_reference_extension : 9;
                    boost::uint16_t pcr_reserved : 6;
                    boost::uint16_t program_clock_reference_base_last1bit : 1;
#endif
                };
                boost::uint16_t pcr;
            };

            // if ( OPCR_flag == 1)
            boost::uint32_t original_program_clock_reference_base; // 33
            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t original_program_clock_reference_base_last1bit : 1;
                    boost::uint16_t opcr_reserved : 6;
                    boost::uint16_t original_program_clock_reference_extension : 9;
#else 
                    boost::uint16_t original_program_clock_reference_extension : 9;
                    boost::uint16_t opcr_reserved : 6;
                    boost::uint16_t original_program_clock_reference_base_last1bit : 1;
#endif
                };
                boost::uint16_t opcr;
            };

            // if (splicing_point_flag == 1)
            boost::uint8_t splice_countdown;
            // not finish 有些标志没有处理
            std::vector<boost::uint8_t> stuffing_bytes;

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & adaptation_field_length;
                boost::int32_t suffering_size = 0;
                if (adaptation_field_length > 0) {
                    ar & flag;
                    suffering_size = adaptation_field_length - 1;

                    if (1 == PCR_flag) {
                        ar & program_clock_reference_base;
                        ar & pcr;
                        suffering_size -= 6;
                    }
                    if (suffering_size > 0) {
                        util::serialization::serialize_collection(ar, stuffing_bytes, suffering_size);
                    }
                }
            }

        };

        struct TransportPacket
        {
            boost::uint8_t  sync_byte;
            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t transport_error_indicator : 1;
                    boost::uint16_t payload_uint_start_indicator : 1;
                    boost::uint16_t transport_priority : 1;
                    boost::uint16_t Pid : 13;
#else
                    boost::uint16_t Pid : 13;
                    boost::uint16_t transport_priority : 1;
                    boost::uint16_t payload_uint_start_indicator : 1;
                    boost::uint16_t transport_error_indicator : 1;
#endif
                };
                boost::uint16_t pid;
            };

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint8_t  transport_scrambling_control : 2;
                    boost::uint8_t  adaptat_field_control : 2;
                    boost::uint8_t  continuity_counter : 4;
#else
                    boost::uint8_t  continuity_counter : 4;
                    boost::uint8_t  adaptat_field_control : 2;
                    boost::uint8_t  transport_scrambling_control : 2;
#endif
                };
                boost::uint8_t control_field;
            };

            TransportPacket()
                : sync_byte(0x47)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & sync_byte;
                ar & pid;
                ar & control_field;
            }
        };

        struct PESPacket
        {
            boost::uint8_t packet_start_code_prefix1;
            boost::uint8_t packet_start_code_prefix2;
            boost::uint8_t packet_start_code_prefix3;
            boost::uint8_t stream_id;
            boost::uint16_t PES_packet_length;

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint8_t reserved : 2;
                    boost::uint8_t PES_scrambling_control : 2;
                    boost::uint8_t PES_priority : 1;
                    boost::uint8_t data_alignment_indicator : 1;
                    boost::uint8_t copyright : 1;
                    boost::uint8_t original_or_copy : 1;
#else
                    boost::uint8_t original_or_copy : 1;
                    boost::uint8_t copyright : 1;
                    boost::uint8_t data_alignment_indicator : 1;
                    boost::uint8_t PES_priority : 1;
                    boost::uint8_t PES_scrambling_control : 2;
                    boost::uint8_t reserved : 2;
#endif
                };
                boost::uint8_t flag1;
            };

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint8_t PTS_DTS_flags : 2;
                    boost::uint8_t ESCR_flag : 1;
                    boost::uint8_t ES_rate_flag : 1;
                    boost::uint8_t DSM_trick_mode_flag : 1;
                    boost::uint8_t additional_copy_info_flag : 1;
                    boost::uint8_t PES_CRC_flag : 1;
                    boost::uint8_t PES_extension_flag : 1;
#else
                    boost::uint8_t PES_extension_flag : 1;
                    boost::uint8_t PES_CRC_flag : 1;
                    boost::uint8_t additional_copy_info_flag : 1;
                    boost::uint8_t DSM_trick_mode_flag : 1;
                    boost::uint8_t ES_rate_flag : 1;
                    boost::uint8_t ESCR_flag : 1;
                    boost::uint8_t PTS_DTS_flags : 2;
#endif
                };
                boost::uint8_t flag2;
            };

            boost::uint8_t PES_header_data_length;

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint8_t reserved1 : 4;
                    boost::uint8_t Pts32_30 : 3;
                    boost::uint8_t pts_marker_bit1 : 1;
#else
                    boost::uint8_t pts_marker_bit1 : 1;
                    boost::uint8_t Pts32_30 : 3;
                    boost::uint8_t reserved1 : 4;
#endif
                };
                boost::uint8_t pts32_30;
            };

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t Pts29_15 : 15;
                    boost::uint8_t pts_marker_bit2 : 1;
#else
                    boost::uint16_t pts_marker_bit2 : 1;
                    boost::uint16_t Pts29_15 : 15;
#endif
                };
                boost::uint16_t pts29_15;
            };

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t Pts14_0 : 15;
                    boost::uint16_t pts_marker_bit3 : 1;
#else
                    boost::uint16_t pts_marker_bit3 : 1;
                    boost::uint16_t Pts14_0 : 15;
#endif
                };
                boost::uint16_t pts14_0;
            };

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint8_t reserved2 : 4;
                    boost::uint8_t Dts32_30 : 3;
                    boost::uint8_t dts_marker_bit1 : 1;
#else
                    boost::uint8_t dts_marker_bit1 : 1;
                    boost::uint8_t Dts32_30 : 3;
                    boost::uint8_t reserved2 : 4;
#endif
                };
                boost::uint8_t dts32_30;
            };

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t Dts29_15 : 15;
                    boost::uint8_t  dts_marker_bit2 : 1;
#else
                    boost::uint16_t dts_marker_bit2 : 1;
                    boost::uint16_t Dts29_15 : 15;
#endif
                };
                boost::uint16_t dts29_15;
            };

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t Dts14_0 : 15;
                    boost::uint16_t dts_marker_bit3 : 1;
#else
                    boost::uint16_t dts_marker_bit3 : 1;
                    boost::uint16_t Dts14_0 : 15;
#endif
                };
                boost::uint16_t dts14_0;
            };

            PESPacket()
                : packet_start_code_prefix1(0)
                , packet_start_code_prefix2(0)
                , packet_start_code_prefix3(1)
                , reserved(2)
                , reserved1(2)
                , reserved2(3)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & packet_start_code_prefix1;
                ar & packet_start_code_prefix2;
                ar & packet_start_code_prefix3;
                ar & stream_id;
                ar & PES_packet_length;
                ar & flag1;
                ar & flag2;
                ar & PES_header_data_length;
                if (PTS_DTS_flags == 2) { // pts
                    ar & pts32_30;
                    ar & pts29_15;
                    ar & pts14_0;
                }
                if (PTS_DTS_flags == 3) { // pts & dts
                    ar & pts32_30;
                    ar & pts29_15;
                    ar & pts14_0;
                    ar & dts32_30;
                    ar & dts29_15;
                    ar & dts14_0;
                }
            }
        };

        struct PSI_table
        {
            boost::uint8_t pointer;
            boost::uint8_t table_id;
            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t section_syntax_indicator : 1;
                    boost::uint16_t undef : 1;
                    boost::uint16_t reserved : 2;
                    boost::uint16_t section_length : 12;
#else
                    boost::uint16_t section_length : 12;
                    boost::uint16_t reserved : 2;
                    boost::uint16_t undef : 1;
                    boost::uint16_t section_syntax_indicator : 1;
#endif
                };
                boost::uint16_t section;
            };
            boost::uint16_t transport_stream_id;

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint8_t reserved1 : 2;
                    boost::uint8_t version_number : 5;
                    boost::uint8_t current_next_indicator : 1;
#else
                    boost::uint8_t current_next_indicator : 1;
                    boost::uint8_t version_number : 5;
                    boost::uint8_t reserved1 : 2;
#endif
                };
                boost::uint8_t info;
            };
            boost::uint8_t section_number;
            boost::uint8_t last_section_number;

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & pointer;
                ar & table_id;
                ar & section;
                ar & transport_stream_id;
                ar & info;
                ar & section_number;
                ar & last_section_number;
            }
        };

        struct PAT_section
        {
            boost::uint16_t program_number;
            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t reserved : 3;
                    boost::uint16_t Pid : 13;
#else
                    boost::uint16_t Pid : 13;
                    boost::uint16_t reserved : 3;
#endif
                };
                boost::uint16_t pid;
            };


            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & program_number;
                ar & pid;
            }
        };

        struct PMT_section
        {
            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t reserved : 3;
                    boost::uint16_t Pcr_Pid : 13;
#else
                    boost::uint16_t Pcr_Pid : 13;
                    boost::uint16_t reserved : 3;
#endif
                };
                boost::uint16_t pcr_pid;
            };

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t reserved1 : 4;
                    boost::uint16_t program_info_length : 12;
#else
                    boost::uint16_t program_info_length : 12;
                    boost::uint16_t reserved1 : 4;
#endif
                };
                boost::uint16_t pil;
            };

            //for (i = 0; i < N; i++) {
            //    descriptor()
            //}

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & pcr_pid;
                ar & pil;
            }
        };

        struct TsStreamInfo
        {
            boost::uint8_t stream_type;
            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t reserved : 3;
                    boost::uint16_t elementary_PID : 13;
#else
                    boost::uint16_t elementary_PID : 13;
                    boost::uint16_t reserved : 3;
#endif
                };
                boost::uint16_t elementary_pid;
            };

            union {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint16_t reserved1 : 4;
                    boost::uint16_t ES_info_length : 12;
#else
                    boost::uint16_t ES_info_length : 12;
                    boost::uint16_t reserved1 : 4;
#endif
                };
                boost::uint16_t es_info_length;
            };

            //for (i = 0; i < N; i++) {
            //    descriptor()
            //}

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & stream_type;
                ar & elementary_pid;
                ar & es_info_length;
            }
        };

    }
}

#endif
