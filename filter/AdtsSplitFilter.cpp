// AdtsSplitFilter.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/filter/AdtsSplitFilter.h"
#include "ppbox/mux/MuxError.h"

#include <ppbox/avformat/codec/aac/AacAdts.h>
#include <ppbox/avformat/stream/BitsIStream.h>
#include <ppbox/avformat/stream/SampleBuffers.h>
#include <ppbox/avformat/Format.h>
using namespace ppbox::avformat;

#include <util/buffers/CycleBuffers.h>

using namespace boost::system;

namespace ppbox
{
    namespace mux
    {

        AdtsSplitFilter::AdtsSplitFilter()
            : audio_track_(boost::uint32_t(-1))
            , is_save_sample_(false)
            , sample_per_frame_(0)
        {
        }

        AdtsSplitFilter::~AdtsSplitFilter()
        {
        }

        bool AdtsSplitFilter::open(
            MediaInfo const & media_info, 
            std::vector<StreamInfo> const & streams, 
            boost::system::error_code & ec)
        {
            if (!Filter::open(media_info, streams, ec))
                return false;
            audio_track_ = boost::uint32_t(-1);
            for (size_t i = 0; i < streams.size(); ++i) {
                if (streams[i].type == StreamType::AUDI) {
                    audio_track_ = i;
                    // ο TimeScaleTransfer
                    scale_.reset_scale(streams[i].audio_format.sample_rate, streams[i].time_scale);
                    sample_per_frame_ = 1024;
                    break;
                }
            }
            return true;
        }

        bool AdtsSplitFilter::get_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (!is_save_sample_) {
                if (!Filter::get_sample(sample, ec))
                    return false;
                if (sample.itrack == audio_track_) {
                    sample_ = sample;
                    is_save_sample_ = true;
                    scale_.last_out(sample.dts);
                } else {
                    return true;
                }
            }
            util::buffers::CycleBuffers<std::deque<boost::asio::const_buffer>, boost::uint8_t> buf(sample_.data);
            buf.commit(sample_.size);
            BitsIStream<boost::uint8_t> bits_is(buf);
            AacAdts adts;
            bits_is >> adts;
            if (adts.frame_length < sample_.size) {
                SampleBuffers::ConstBuffers data, data1, data2;
                sample_.data.swap(data);
                SampleBuffers::BuffersPosition beg(data.begin(), data.end());
                SampleBuffers::BuffersPosition end(data.end());
                SampleBuffers::BuffersPosition pos(beg);
                pos.increment_bytes(end, adts.frame_length);
                data1.insert(data1.end(), SampleBuffers::range_buffers_begin(beg, pos), SampleBuffers::range_buffers_end());
                data2.insert(data2.end(), SampleBuffers::range_buffers_begin(pos, end), SampleBuffers::range_buffers_end());
                sample.slice(sample_);
                sample.data.swap(data1);
                sample_.data.swap(data2);
                sample.size = adts.frame_length;
                sample_.size -= sample.size;
                sample_.dts = scale_.get();
                scale_.inc(sample_per_frame_);
            } else {
                sample = sample_;
                is_save_sample_ = false;
            }
            return true;
        }

        bool AdtsSplitFilter::before_seek(
            Sample & sample, 
            boost::system::error_code & ec)
        {
            sample.append(sample_);
            is_save_sample_ = false;
            return Filter::before_seek(sample, ec);
        }

    } // namespace mux
} // namespace ppbox
