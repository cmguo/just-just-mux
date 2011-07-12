#include "ppbox/mux/Common.h"
#include "ppbox/mux/TsSegmentCreater.h"
#include "ppbox/mux/ts/TsMux.h"

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {
        TsSegmentCreater::TsSegmentCreater(boost::uint32_t seg_duration)
            : segment_duration_(seg_duration)
            , first_idr_timestamp_(0)
            , segment_sum_(0)
            , segment_state_(SegmentType::NONE)
            , ts_mux_(new TsMux)
        {
        }

        TsSegmentCreater::~TsSegmentCreater()
        {
            if (ts_mux_) {
                delete ts_mux_;
                ts_mux_ = NULL;
            }
        }

        error_code TsSegmentCreater::open(
            demux::Demuxer * demuxer,
            error_code & ec)
        {
            ts_mux_->open(demuxer, ec);
            if (!ec) {
                ts_mux_->set_ipad(true);
            }
            return ec;
        }

        error_code TsSegmentCreater::read(
            MuxTagEx & tag,
            error_code & ec)
        {
            if (segment_state_ == SegmentType::END) {
                ec.clear();
                segment_state_ = SegmentType::BEGAN;
                tag = tag_;
                tag.tag_header_buffer = ts_mux_->get_head(tag.tag_header_length);
            } else {
                ts_mux_->readex(&tag_, ec);
                if (!ec) {
                    if (segment_state_ == SegmentType::NONE) {
                        if (tag_.is_sync
                            && tag_.itrack == ts_mux_->video_track_index()) {
                                first_idr_timestamp_ = tag_.time;
                                segment_sum_++;
                                segment_state_ = SegmentType::BEGAN;
                                tag = tag_;
                                tag.tag_header_buffer = ts_mux_->get_head(tag.tag_header_length);
                        } else {
                            ec = error::mux_invalid_sample;
                        }
                    } else if (segment_state_ == SegmentType::BEGAN) {
                        if (tag_.is_sync
                            && tag_.itrack == ts_mux_->video_track_index()
                            && (tag_.time - first_idr_timestamp_) >= (segment_sum_*segment_duration_)) {
                                segment_sum_++;
                                segment_state_ = SegmentType::END;
                                ec = error::mux_segment_end;
                        } else {
                            tag = tag_;
                        }
                    }
                }
            }
            return ec;
        }

        boost::uint32_t & TsSegmentCreater::segment_duration(void)
        {
            return segment_duration_;
        }

        void TsSegmentCreater::close(void)
        {
            segment_state_ = SegmentType::NONE;
            ts_mux_->close();
        }
    } // namespace mux
} // namespace ppbox
