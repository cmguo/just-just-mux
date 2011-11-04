// TsMux.h

#ifndef _PPBOX_MUX_TS_TS_MUX_H_
#define _PPBOX_MUX_TS_TS_MUX_H_

#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/filter/KeyFrameFilter.h"
#include "ppbox/mux/ts/Mpeg2Ts.h"

namespace ppbox
{
    namespace mux
    {
        class TsMux
            : public Muxer
        {
        public:
            TsMux()
                : is_merge_audio_(false)
                , header_(new AP4_MemoryByteStream(512))
                , pat_(new Stream(0))
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
                Config().register_module("TsMux")
                    << CONFIG_PARAM_NAME_RDWR("merge_audio", is_merge_audio_);
            }

            virtual ~TsMux()
            {
                if (header_) {
                    header_->Release();
                }

                if (pat_) {
                    delete pat_;
                    pat_ = NULL;
                }

                if (pmt_) {
                    delete pmt_;
                    pmt_ = NULL;
                }
            }

        public:
            void add_stream(
                ppbox::demux::MediaInfo & mediainfo,
                std::vector<Transfer *> & transfer);

            void head_buffer(
                ppbox::demux::Sample & tag);

        private:
            void WritePAT(AP4_ByteStream & output);
            void WritePMT(AP4_ByteStream & output);

        private:
            bool is_merge_audio_;
            AP4_MemoryByteStream * header_;
            Stream * pat_;
            Stream * pmt_;
            bool has_audio_;
            bool has_video_;
            boost::uint16_t audio_pid_;
            boost::uint16_t video_pid_;
            boost::uint16_t audio_stream_id_;
            boost::uint16_t video_stream_id_;
            boost::uint8_t audio_stream_type_;
            boost::uint8_t video_stream_type_;
        };
    }
}

#endif // _PPBOX_MUX_TS_TS_MUX_H_
