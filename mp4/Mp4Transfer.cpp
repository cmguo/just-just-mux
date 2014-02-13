// Mp4Transfer.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/mp4/Mp4Transfer.h"
#include "ppbox/mux/mp4/Mp4DataContext.h"

#include <ppbox/avformat/mp4/Mp4Format.h>
using namespace ppbox::avformat;
#include <ppbox/avbase/object/Object.hpp>
using namespace ppbox::avbase;

#include <framework/system/BytesOrder.h>
using namespace framework::system;

namespace ppbox
{
    namespace mux
    {

        Mp4Transfer::Mp4Transfer(
            Mp4Track * track, 
            Mp4DataContext * ctx)
            : track_(track)
            , table_(&track_->sample_table())
            , ctx_(ctx)
        {
        }

        Mp4Transfer::~Mp4Transfer()
        {
        }

        void Mp4Transfer::transfer(
            StreamInfo & info)
        {
            Mp4Format mp4;
            boost::system::error_code ec;
            CodecInfo const * codec = mp4.codec_from_codec(info.type, info.sub_type, ec);
            if (codec == NULL) {
                return;
            }

            track_->timescale(info.time_scale);
            track_->duration(info.duration);
            Mp4SampleEntry * entry = table_->description_table().create_description(codec->stream_type);
            if (info.type == StreamType::VIDE) {
                track_->width(info.video_format.width);
                track_->height(info.video_format.height);
                Mp4VisualSampleEntry & visual = static_cast<Mp4VisualSampleEntry &>(*entry);
                visual.width(info.video_format.width);
                visual.height(info.video_format.height);
            } else {
                Mp4AudioSampleEntry & audio = static_cast<Mp4AudioSampleEntry &>(*entry);
                audio.channel_count(info.audio_format.channel_count);
                audio.sample_size(info.audio_format.sample_size);
                audio.sample_rate(info.audio_format.sample_rate);
            }

            if (codec->context) {
                if ((intptr_t)codec->context < 256) {
                    Mp4EsDescription * es_desc = entry->create_es_description();
                    Mp4DecoderConfigDescriptor * config = es_desc->decoder_config();
                    config->ObjectTypeIndication = (boost::uint8_t)(intptr_t)codec->context;
                    if (info.type == StreamType::VIDE) {
                        config->StreamType = MpegStreamType::VISUAL;
                    } else {
                        config->StreamType = MpegStreamType::AUDIO;
                    }
                    config->AverageBitrate = info.bitrate;
                    Mp4DecoderSpecificInfoDescriptor * dinfo = es_desc->decoder_info();
                    dinfo->Info = info.format_data;
                } else {
                    if (!info.format_data.empty()) {
                        Mp4Box * config = entry->create_item((boost::uint32_t)(intptr_t)codec->context);
                        config->raw_data(Mp4Box::raw_data_t(&info.format_data.at(0), info.format_data.size()));
                    }
                }
            }
        }

        void Mp4Transfer::transfer(
            Sample & sample)
        {
            ctx_->put_sample(*table_, sample);
        }

        void Mp4Transfer::on_event(
            MuxEvent const & event)
        {
            if (event.type == event.end) {
                table_->put_eos();
            }
        }

    } // namespace mux
} // namespace ppbox
