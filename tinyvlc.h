//tinyvlc.h
#ifndef _PPBOX_MUX_TINY_VLC_H_
#define _PPBOX_MUX_TINY_VLC_H_

#include "ppbox/mux/elements.h"
#include "ppbox/mux/detail/BitsReader.h"

namespace ppbox
{
    namespace mux
    {
        struct bit_stream
        {
            boost::int32_t   read_len;
            boost::int32_t   code_len;
            boost::int32_t   frame_bitoffset;
            boost::int32_t   bitstream_length;
            boost::uint8_t   *streamBuffer;
            boost::int32_t   ei_flag;
        };
        // typedef struct bit_stream Bitstream;
        typedef MyBitsReader Bitstream;

        class NaluParser
        {
        public:
            NaluParser()
                : got_sps_(false)
                , got_pps_(false)
                , is_ready(false)
                , pic_order_cnt_lsb(0)
            {
            }

            ~NaluParser()
            {
            }

            void parse_sps(Bitstream & bs);

            void parse_pps(Bitstream & bs);

            void parse_frame(Bitstream & bs);

        public:
            bool got_sps_;
            bool got_pps_;
            bool is_ready;
            Nal_header cur_nal_header_;

            // SPS struct
            char profile_idc;
            char useless1;
            char level_idc;
            boost::int32_t sps_seq_parameter_set_id;
            boost::int32_t chroma_format_idc;
            bool residual_colour_transform_flag;
            boost::int32_t bit_depth_luma_minus8;
            boost::int32_t bit_depth_chroma_minus8;
            bool qpprime_y_zero_transform_bypass_flag;
            bool seq_scaling_matrix_present_flag;
            bool  seq_scaling_list_present_flag[8];
            boost::int32_t    delta_scale;
            boost::int32_t log2_max_frame_num_minus4;
            boost::int32_t pic_order_cnt_type;

            boost::int32_t log2_max_pic_order_cnt_lsb_minus4;
            bool  delta_pic_order_always_zero_flag;
            boost::int32_t offset_for_non_ref_pic;
            boost::int32_t offset_for_top_to_bottom_field;
            boost::int32_t num_ref_frames_in_pic_order_cnt_cycle;
            boost::int32_t offset_for_ref_frame[100];
            boost::int32_t num_ref_frames;
            bool  gaps_in_frame_num_value_allowed_flag;
            boost::int32_t pic_width_in_mbs_minus1;
            boost::int32_t pic_height_in_map_units_minus1;
            bool  frame_mbs_only_flag;
            bool  mb_adaptive_frame_field_flag;
            bool  direct_8x8_inference_flag;
            bool  frame_cropping_flag;
            boost::int32_t frame_crop_left_offset;
            boost::int32_t frame_crop_right_offset;
            boost::int32_t frame_crop_top_offset;
            boost::int32_t frame_crop_bottom_offset;
            bool  vui_parameters_present_flag;

            // pic_parameter_set_rbsp
            boost::int32_t pps_pic_parameter_set_id;
            boost::int32_t pps_seq_parameter_set_id;
            bool  entropy_coding_mode_flag;
            bool  pic_order_present_flag;
            boost::int32_t num_slice_groups_minus1;
            boost::int32_t slice_group_map_type;
            boost::int32_t run_length_minus1;
            boost::int32_t top_left;
            boost::int32_t bottom_right;
            bool  slice_group_change_direction_flag;
            boost::int32_t slice_group_change_rate_minus1;
            boost::int32_t pic_size_in_map_units_minus1;
            boost::int32_t slice_group_id;
            boost::int32_t num_ref_idx_l0_active_minus1;
            boost::int32_t num_ref_idx_l1_active_minus1;
            bool  weighted_pred_flag;
            char weighted_bipred_idc;
            boost::int32_t pic_init_qp_minus26;
            boost::int32_t pic_init_qs_minus26;
            boost::int32_t chroma_qp_index_offset;
            bool  deblocking_filter_control_present_flag; // u(1)
            bool  constrained_intra_pred_flag;
            bool  redundant_pic_cnt_present_flag;

            // slice_layer_without_partitioning_rbsp
            // slice_header
            boost::int32_t first_mb_in_slice;
            boost::int32_t    slice_type;
            boost::int32_t sh_pic_parameter_set_id;
            boost::int32_t frame_num;
            bool  field_pic_flag;
            bool  bottom_field_flag;
            boost::int32_t idr_pic_id;
            boost::int32_t pic_order_cnt_lsb;
            boost::int32_t delta_pic_order_cnt_bottom;
            boost::int32_t delta_pic_order_cnt[2];
            boost::int32_t redundant_pic_cnt;
            // vui parameters
            bool  aspect_ratio_info_present_flag;
            boost::int32_t aspect_ratio_idc;
            boost::int32_t sar_width;
            boost::int32_t sar_height;
            bool  overscan_info_present_flag;
            bool  overscan_appropriate_flag;
            bool  video_signal_type_present_flag;
            boost::int32_t video_format;
            bool  video_full_range_flag;
            bool  colour_description_present_flag;
            boost::int32_t colour_primaries;
            boost::int32_t transfer_characteristics;
            boost::int32_t matrix_coefficients;
            bool  chroma_loc_info_present_flag;
            boost::int32_t chroma_sample_loc_type_top_field;
            boost::int32_t chroma_sample_loc_type_bottom_field;
            bool  timing_info_present_flag;
            boost::uint32_t num_units_in_tick;
            boost::uint32_t time_scale;
            bool  fixed_frame_rate_flag;
            bool nal_hrd_parameters_present_flag;
            bool vcl_hrd_parameters_present_flag;
            bool pic_struct_present_flag;
            bool bitstream_restriction_flag;
        };
    } // namespace mux
} // namespace ppbox

#endif
