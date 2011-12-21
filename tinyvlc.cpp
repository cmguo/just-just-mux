//tinyvlc.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/tinyvlc.h"

#include <framework/system/BytesOrder.h>
#include <math.h>
//#include <iostream>

namespace ppbox
{
    namespace mux
    {

        void linfo_ue(boost::int32_t len, boost::int32_t info, boost::int32_t *value1)
        {
            assert((len >> 1) < 32);
            *value1 = (boost::int32_t) (((boost::uint32_t) 1 << (len >> 1)) + (boost::uint32_t) (info) - 1);
        }

        void linfo_se(boost::int32_t len, boost::int32_t info, boost::int32_t *value1)
        {
            assert ((len >> 1) < 32);
            boost::uint32_t n = ((boost::uint32_t) 1 << (len >> 1)) + (boost::uint32_t) info - 1;
            *value1 = (n + 1) >> 1;
            if((n & 0x01) == 0)
                *value1 = -*value1;
        }

        boost::int32_t ue_v (Bitstream & bitstream)
        {
            boost::uint32_t len = 0;
            boost::uint32_t v1 = bitstream.read_bits_vlc(len);
            boost::int32_t v2;
            linfo_ue(len, v1, &v2);
            return v2;
        }

        boost::int32_t se_v (Bitstream & bitstream)
        {
            boost::uint32_t len = 0;
            boost::uint32_t v1 = bitstream.read_bits_vlc(len);
            boost::int32_t v2;
            linfo_se(len, v1, &v2);
            return v2;
        }

        boost::int32_t u_v (boost::int32_t LenInBits, Bitstream & bitstream)
        {
            return bitstream.read_bits_flc(LenInBits);
        }

        bool u_1 (Bitstream & bitstream)
        {
            return (bool) u_v (1, bitstream);
        }

        boost::int32_t next_bits(Bitstream & bitstream, boost::int32_t LenInBits, boost::int32_t* info)
        {
            *info = (boost::int32_t)bitstream.read_bits_flc(LenInBits);
            return bitstream.failed() ? 0 : LenInBits;
        }

        #define LengthInBits(x) ((x << 3) + 7)

        #define pass_in_bits(x) bs.read_bits_flc(x);
        #define parse_ue_v(x) x = ue_v(bs)
        #define parse_se_v(x) x = se_v(bs)
        #define parse_u_v(x, y) x = u_v(y, bs)
        #define parse_u_x(x, y) x = u_v(y, bs);
        #define parse_u_1(x) x = u_1(bs);

        void NaluParser::parse_sps(Bitstream & bs)
        {
            // skip slice type
            bs.read_bits_flc(8);

            profile_idc = bs.read_bits_flc(8);
            useless1 = bs.read_bits_flc(8);
            level_idc = bs.read_bits_flc(8);
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

        void NaluParser::parse_pps(Bitstream & bs)
        {
            // skip slice type
            bs.read_bits_flc(8);

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

        void NaluParser::parse_frame(Bitstream & bs)
        {
            //assert(got_pps_ && got_sps_);
            // skip slice type
            boost::uint8_t nalu_type = (boost::uint8_t)bs.read_bits_flc(8);
            cur_nal_header_ = *(Nal_header *)&nalu_type;

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
            is_ready = true;
        }

    } // namespace mux
} // namespace ppbox
