//tinyvlc.cpp
#include "ppbox/mux/Common.h"
#include "ppbox/mux/ts/tinyvlc.h"

#include <framework/system/BytesOrder.h>

#include <math.h>
//#include <iostream>

namespace ppbox
{
    namespace mux
    {
        typedef struct syntaxelement
        {
            boost::int32_t           type;
            boost::int32_t           value1;
            boost::int32_t           value2;
            boost::int32_t           len;
            boost::int32_t           inf;
            boost::uint32_t          bitpattern;
            boost::int32_t           context;
            boost::int32_t           k;

        #if TRACE
        #define       TRACESTRING_SIZE 100           //!< size of trace string
            char          tracestring[TRACESTRING_SIZE]; //!< trace string
        #endif
            void  (*mapping)(boost::int32_t len, boost::int32_t info, boost::int32_t *value1, boost::int32_t *value2);
        } SyntaxElement;


        #define SYMTRACESTRING
        void linfo_ue(boost::int32_t len, boost::int32_t info, boost::int32_t *value1, boost::int32_t *dummy)
        {
            //assert ((len >> 1) < 32);
            *value1 = (boost::int32_t) (((boost::uint32_t) 1 << (len >> 1)) + (boost::uint32_t) (info) - 1);
        }

        void linfo_se(boost::int32_t len, boost::int32_t info, boost::int32_t *value1, boost::int32_t *dummy)
        {
            //assert ((len >> 1) < 32);
            boost::uint32_t n = ((boost::uint32_t) 1 << (len >> 1)) + (boost::uint32_t) info - 1;
            *value1 = (n + 1) >> 1;
            if((n & 0x01) == 0)
                *value1 = -*value1;
        }

        boost::int32_t GetVLCSymbol (boost::uint8_t buffer[], boost::int32_t totbitoffset, boost::int32_t *info, boost::int32_t bytecount)
        {
            boost::int32_t byteoffset = (totbitoffset >> 3);
            boost::int32_t  bitoffset  = (7 - (totbitoffset & 0x07));
            boost::int32_t  bitcounter = 1;
            boost::int32_t  len        = 0;
            boost::uint8_t *cur_byte  = &(buffer[byteoffset]);
            boost::int32_t  ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;

            while (ctr_bit == 0) {
                len++;
                bitcounter++;
                bitoffset--;
                bitoffset &= 0x07;
                cur_byte  += (bitoffset == 7);
                byteoffset+= (bitoffset == 7);
                ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;
            }

            if (byteoffset + ((len + 7) >> 3) > bytecount)
                return -1;
            else {
                boost::int32_t inf = 0;
                while (len--) {
                    bitoffset --;
                    bitoffset &= 0x07;
                    cur_byte  += (bitoffset == 7);
                    bitcounter++;
                    inf <<= 1;
                    inf |= ((*cur_byte) >> (bitoffset)) & 0x01;
                }
                *info = inf;
                return bitcounter;
            }
        }

        boost::int32_t GetBits (
            boost::uint8_t buffer[],
            boost::int32_t totbitoffset,
            boost::int32_t *info,
            boost::int32_t bitcount,
            boost::int32_t numbits)
        {
            if ((totbitoffset + numbits ) > bitcount) {
                return -1;
            } else {
                boost::int32_t bitoffset  = 7 - (totbitoffset & 0x07);
                boost::int32_t byteoffset = (totbitoffset >> 3);
                boost::int32_t bitcounter = numbits;
                boost::uint8_t *curbyte  = &(buffer[byteoffset]);
                boost::int32_t inf = 0;

                while (numbits--) {
                    inf <<=1;
                    inf |= ((*curbyte)>> (bitoffset--)) & 0x01;
                    if (bitoffset == -1 ) {
                        curbyte++;
                        bitoffset = 7;
                    }
                }
                *info = inf;
                return bitcounter;
            }
        }

        boost::int32_t readSyntaxElement_VLC(SyntaxElement *sym, Bitstream *currStream)
        {
            sym->len =  GetVLCSymbol(
                currStream->streamBuffer,
                currStream->frame_bitoffset,
                &(sym->inf), currStream->bitstream_length);
            if (sym->len == -1)
                return -1;
            currStream->frame_bitoffset += sym->len;
            sym->mapping(sym->len, sym->inf, &(sym->value1), &(sym->value2));
            return 1;
        }

        boost::int32_t readSyntaxElement_FLC(SyntaxElement *sym, Bitstream *currStream)
        {
            boost::int32_t BitstreamLengthInBits  = (currStream->bitstream_length << 3) + 7;
            if (GetBits(
                currStream->streamBuffer,
                currStream->frame_bitoffset,
                &(sym->inf),
                BitstreamLengthInBits,
                sym->len) < 0) {
                return -1;
            }

            sym->value1 = sym->inf;
            currStream->frame_bitoffset += sym->len;
            return 1;
        }

        boost::int32_t ue_v (char *tracestring, Bitstream *bitstream)
        {
            SyntaxElement symbol;
            symbol.type = SE_HEADER;
            symbol.mapping = linfo_ue;
            // SYMTRACESTRING(tracestring);
            readSyntaxElement_VLC (&symbol, bitstream);
            // p_Dec->UsedBits+=symbol.len;
            return symbol.value1;
        }

        boost::int32_t se_v (char *tracestring, Bitstream *bitstream)
        {
            SyntaxElement symbol;
            //assert (bitstream->streamBuffer != NULL);
            symbol.type = SE_HEADER;
            symbol.mapping = linfo_se;
            readSyntaxElement_VLC (&symbol, bitstream);
            // p_Dec->UsedBits+=symbol.len;
            return symbol.value1;
        }

        boost::int32_t u_v (boost::int32_t LenInBits, char* tracestring, Bitstream *bitstream)
        {
            SyntaxElement symbol;
            symbol.inf = 0;
            //assert (bitstream->streamBuffer != NULL);
            symbol.type = SE_HEADER;
            symbol.mapping = linfo_ue;
            symbol.len = LenInBits;
            readSyntaxElement_FLC (&symbol, bitstream);
            // p_Dec->UsedBits+=symbol.len;
            return symbol.inf;
        }

        bool u_1 (char *tracestring, Bitstream *bitstream)
        {
            return (bool) u_v (1, tracestring, bitstream);
        }

        boost::int32_t next_bits(Bitstream* bitstream, boost::int32_t LenInBits, boost::int32_t* info)
        {
            return GetBits(
                bitstream->streamBuffer,
                bitstream->frame_bitoffset,
                info,
                (bitstream->bitstream_length << 3) + 7,
                LenInBits);
        }


        #define LengthInBits(x) ((x << 3) + 7)
        inline bool CheckDataEnoughInBits(Bitstream* bs, boost::int32_t lenInBits)
        {
            return ((bs->frame_bitoffset + lenInBits)  <= LengthInBits(bs->bitstream_length));
        };

        #define pass_in_bits(x) bs->frame_bitoffset += x;
        #define parse_ue_v(x) x = ue_v(0, bs)
        #define parse_se_v(x) x = se_v(0, bs)
        #define parse_u_v(x, y) x = u_v(y, 0, bs)
        //#define parse_u_1(x) next_bits(bs, 1, (boost::int32_t*)&x); pass_in_bits(1);
        //#define parse_u_x(x, y) next_bits(bs, y, (boost::int32_t*)&x); pass_in_bits(y);
        #define parse_u_x(x, y) x = u_v(y, 0, bs);
        #define parse_u_1(x) x = u_1(0, bs);

        void NaluParser::parse( Bitstream* bs )
        {
            is_ready_ = false;
            do {
                // parse start_code_prefix
                bool got_start_code(false);
                boost::int32_t start_code(0);
                bs->frame_bitoffset = (bs->frame_bitoffset & 0xfffffff8) + (bool(bs->frame_bitoffset & 0x07) << 3);
                do {
                    if (next_bits(bs, 24, &start_code) != 24)
                        break;
                    if (start_code == 0x000001) {
                        bs->frame_bitoffset += 24;
                        got_start_code = true;
                        break;
                    }
                    if (next_bits(bs, 32, &start_code) != 32)
                        break;
                    if (start_code == 0x00000001) {
                        bs->frame_bitoffset += 32;
                        got_start_code = true;
                        break;
                    }
                    bs->frame_bitoffset += 8;
                } while (bs->frame_bitoffset < LengthInBits(bs->bitstream_length));

                if (got_start_code == false)
                    return;
                // parse nal_header
                if ((bs->frame_bitoffset + 8 ) > LengthInBits(bs->bitstream_length))
                    return;
                cur_nal_header_ = *(Nal_header*)(bs->streamBuffer + (bs->frame_bitoffset >> 3));
                bs->frame_bitoffset += 8;

                switch (cur_nal_header_.nal_unit_type) {
                    case 7: // SPS
                    {
                        if (!CheckDataEnoughInBits(bs, 24)) 
                            return;
                        profile_idc = *(char*)(bs->streamBuffer + (bs->frame_bitoffset >> 3));
                        useless1 = *(char*)(bs->streamBuffer + (bs->frame_bitoffset >> 3) + 1);
                        level_idc = *(char*)(bs->streamBuffer + (bs->frame_bitoffset >> 3) + 2);
                        pass_in_bits(24);
                        parse_ue_v(sps_seq_parameter_set_id);
                        if (profile_idc == 100 || profile_idc == 110
                            || profile_idc == 122 || profile_idc == 144) {
                            parse_ue_v(chroma_format_idc);
                            if (chroma_format_idc == 3) {
                                pass_in_bits(1);
                            }
                            parse_ue_v(bit_depth_luma_minus8);
                            parse_ue_v(bit_depth_chroma_minus8);
                            parse_u_1(qpprime_y_zero_transform_bypass_flag);
                            parse_u_1(seq_scaling_matrix_present_flag);
                            if (seq_scaling_matrix_present_flag) {
                                for (boost::int32_t i(0); i < 8; ++i) {
                                    parse_u_1(seq_scaling_list_present_flag[i]);
                                    if (seq_scaling_list_present_flag[i]) {
                                        if (i < 6) {
                                            boost::int32_t lastScale(8), nextScale(8);
                                            for (boost::int32_t j(0); j < 16; ++j) {
                                                if (nextScale != 0) {
                                                    parse_se_v(delta_scale);
                                                    nextScale = ( lastScale + delta_scale + 256 ) % 256;
                                                }
                                                lastScale = ( nextScale  ==  0 ) ? lastScale : nextScale;
                                            }
                                        } else {
                                            boost::int32_t lastScale(8), nextScale(8);
                                            for (boost::int32_t j(0); j < 64; ++j) {
                                                if (nextScale != 0) {
                                                    parse_se_v(delta_scale);
                                                    nextScale = ( lastScale + delta_scale + 256 ) % 256;
                                                }
                                                lastScale = ( nextScale  ==  0 ) ? lastScale : nextScale;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        parse_ue_v(log2_max_frame_num_minus4);
                        parse_ue_v(pic_order_cnt_type);

                        if (pic_order_cnt_type == 0) {
                            parse_ue_v(log2_max_pic_order_cnt_lsb_minus4);
                        } else if (pic_order_cnt_type == 1) {
                            parse_u_1(delta_pic_order_always_zero_flag);
                            parse_se_v(offset_for_non_ref_pic);
                            parse_se_v(offset_for_top_to_bottom_field);
                            parse_ue_v(num_ref_frames_in_pic_order_cnt_cycle); 
                            for( boost::int32_t i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; ++i ) {
                                parse_se_v(offset_for_ref_frame[i]);
                            }
                        }
                        parse_ue_v(num_ref_frames);
                        parse_u_1(gaps_in_frame_num_value_allowed_flag);
                        parse_ue_v(pic_width_in_mbs_minus1);
                        parse_ue_v(pic_height_in_map_units_minus1);
                        parse_u_1(frame_mbs_only_flag);
                        if (!frame_mbs_only_flag) {
                            parse_u_1(mb_adaptive_frame_field_flag);
                        }
                        parse_u_1(direct_8x8_inference_flag);
                        parse_u_1(frame_cropping_flag);
                        if (frame_cropping_flag) {
                            parse_ue_v(frame_crop_left_offset);
                            parse_ue_v(frame_crop_right_offset);
                            parse_ue_v(frame_crop_top_offset);
                            parse_ue_v(frame_crop_bottom_offset);
                        }
                        parse_u_1(vui_parameters_present_flag);
                        if(vui_parameters_present_flag) {
                            matrix_coefficients = 2;
                            parse_u_1(aspect_ratio_info_present_flag);
                            if (aspect_ratio_info_present_flag) {
                                parse_u_x(aspect_ratio_idc, 8);
                                if (aspect_ratio_idc == 255) {
                                    parse_u_x(sar_width, 16);
                                    parse_u_x(sar_height, 16);
                                }
                            }
                            parse_u_1(overscan_info_present_flag);
                            if (overscan_info_present_flag) {
                                parse_u_1(overscan_appropriate_flag);
                            }
                            parse_u_1(video_signal_type_present_flag);
                            if (video_signal_type_present_flag) {
                                parse_u_x(video_format, 3);
                                parse_u_1(video_full_range_flag);
                                parse_u_1(colour_description_present_flag);
                                if (colour_description_present_flag) {
                                    parse_u_x(colour_primaries, 8);
                                    parse_u_x(transfer_characteristics, 8);
                                    parse_u_x(matrix_coefficients, 8);
                                }
                            }
                            parse_u_1(chroma_loc_info_present_flag);
                            if (chroma_loc_info_present_flag) {
                                parse_ue_v(chroma_sample_loc_type_top_field);
                                parse_ue_v(chroma_sample_loc_type_bottom_field);
                            }
                            parse_u_1(timing_info_present_flag);
                            if (timing_info_present_flag) {
                                parse_u_x(num_units_in_tick, 32);
                                parse_u_x(time_scale, 32);
                                parse_u_1(fixed_frame_rate_flag);
                            }
                            parse_u_1(nal_hrd_parameters_present_flag);
                            //parse_u_1(vcl_hrd_parameters_present_flag);
                            //parse_u_1(pic_struct_present_flag);
                            //parse_u_1(bitstream_restriction_flag);
                        }
                        got_sps_ = true;
                    }
                    break;
                    case 8: // pps
                    {
                        parse_ue_v(pps_pic_parameter_set_id);
                        parse_ue_v(pps_seq_parameter_set_id);
                        parse_u_1(entropy_coding_mode_flag);
                        parse_u_1(pic_order_present_flag);
                        parse_ue_v(num_slice_groups_minus1);
                        if( num_slice_groups_minus1 > 0 ) {
                            parse_ue_v(slice_group_map_type);
                            if( slice_group_map_type  ==  0 ) {
                                for( boost::int32_t iGroup = 0; iGroup <= num_slice_groups_minus1; ++iGroup )
                                    parse_ue_v(run_length_minus1);
                            } else if( slice_group_map_type  ==  2 ) {
                                for( boost::int32_t iGroup = 0; iGroup < num_slice_groups_minus1; ++iGroup ) {
                                    parse_ue_v(top_left/*[iGroup]*/); 
                                    parse_ue_v(bottom_right/*[iGroup]*/);
                                }
                            } else if(  slice_group_map_type == 3 || 
                                slice_group_map_type == 4 || 
                                slice_group_map_type == 5 ) {
                                parse_u_1(slice_group_change_direction_flag);
                                parse_ue_v(slice_group_change_rate_minus1);
                            } else if( slice_group_map_type == 6 ) {
                                parse_ue_v(pic_size_in_map_units_minus1);
                                for(boost::int32_t i = 0; i <= pic_size_in_map_units_minus1; i++) {
                                    parse_u_v(slice_group_id, ceil( log( (double)num_slice_groups_minus1 + 1 ) / log((double)2) )/*[ i ]*/);
                                }
                            }
                        }

                        parse_ue_v(num_ref_idx_l0_active_minus1);
                        parse_ue_v(num_ref_idx_l1_active_minus1);
                        pass_in_bits(3);
                        parse_se_v(pic_init_qp_minus26);
                        parse_se_v(pic_init_qs_minus26);
                        parse_se_v(chroma_qp_index_offset);
                        parse_u_1(deblocking_filter_control_present_flag);
                        parse_u_1(constrained_intra_pred_flag);
                        parse_u_1(redundant_pic_cnt_present_flag);
                        got_pps_ = true;
                    }
                    break;
                    case 1: // non-IDR
                    case 5: // IDR
                    {
                        if (!got_sps_
                            || (pic_order_cnt_type != 0 && !got_sps_)) {
                            return;
                        }
                        parse_ue_v(first_mb_in_slice);
                        parse_ue_v(slice_type);
                        parse_ue_v(sh_pic_parameter_set_id);
                        parse_u_v(frame_num, log2_max_frame_num_minus4 + 4);
                        if (!frame_mbs_only_flag) {
                            parse_u_1(field_pic_flag);
                            if (field_pic_flag) {
                                parse_u_1(bottom_field_flag);
                            }
                        }

                        if( cur_nal_header_.nal_unit_type  ==  5 )
                            parse_ue_v(idr_pic_id);
                        if( pic_order_cnt_type  ==  0 ) {
                            parse_u_v(pic_order_cnt_lsb, log2_max_pic_order_cnt_lsb_minus4 + 4);
                            //if (slice_type == 7) {
                            //    std::cout << "I frame [" << pic_order_cnt_lsb << "]" << std::endl;
                            //} else if (slice_type == 5) {
                            //    std::cout << "P frame [" << pic_order_cnt_lsb << "]" << std::endl;
                            //} else {
                            //    std::cout << "B frame [" << pic_order_cnt_lsb << "]" << std::endl;
                            //}
                            if( pic_order_present_flag &&  !field_pic_flag )
                                parse_se_v(delta_pic_order_cnt_bottom);
                        }
                        if( pic_order_cnt_type == 1 && !delta_pic_order_always_zero_flag )
                        {
                            parse_se_v(delta_pic_order_cnt[0]);
                            if( pic_order_present_flag  &&  !field_pic_flag )
                                parse_se_v(delta_pic_order_cnt[1]);
                        }
                        if( redundant_pic_cnt_present_flag )
                            parse_ue_v(redundant_pic_cnt);
                        is_ready_ = true;
                    }
                    break;
                default:
                    break;
                }
            } while (!is_ready_);
        }
    } // namespace mux
} // namespace ppbox
