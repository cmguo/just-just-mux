//MkvTransfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/mkv/MkvTransfer.h"

using namespace ppbox::avformat;
using namespace ppbox::demux;

namespace ppbox
{
    namespace mux
    {

        MkvTransfer::MkvTransfer()
            : add_cluster_flag_(0)
            , time_code_(0)
        {
        }

        MkvTransfer::~MkvTransfer()
        {
        }

        void MkvTransfer::transfer(
            ppbox::demux::MediaInfo & mediainfo)
        {
        }

        void MkvTransfer::transfer(
            ppbox::demux::Sample & sample)
        {
            block_head_buf_.reset();
            MKVOArchive oar(block_head_buf_);

            if (0 == add_cluster_flag_) {
                MKV_Cluster cluster;
                cluster.size = framework::system::VariableNumber<boost::uint32_t>::unknown(1);
                time_code_ = sample.time 
                    + (boost::uint64_t)sample.cts_delta * 1000 / sample.media_info->time_scale;
                cluster.TimeCode = time_code_;
                oar << cluster;
                add_cluster_flag_ = 100;
            }

            MKV_Simple_Block simple_block;
            simple_block.TrackNumber = sample.itrack + 1;
            simple_block.TimeCode= sample.time 
                + (boost::uint64_t)sample.cts_delta * 1000 / sample.media_info->time_scale
                - time_code_;
            if (Sample::sync & sample.flags)
                simple_block.Keyframe = 1;
            else
                simple_block.Keyframe = 0;
            simple_block.size = simple_block.data_size() + sample.size;
            oar << simple_block;

            sample.data.push_front(block_head_buf_.data());
            add_cluster_flag_--;
        }

        void MkvTransfer::on_seek()
        {
            block_head_buf_.reset();
            add_cluster_flag_ = 0;
            time_code_ = 0;
        }
    }

}

