// RawFormat.h

#ifndef _PPBOX_MUX_RAW_RAW_FORMAT_H_
#define _PPBOX_MUX_RAW_RAW_FORMAT_H_

#include <ppbox/avformat/Format.h>

namespace ppbox
{
    namespace mux
    {

        class RawMuxer;

        class RawFormat
            : public ppbox::avformat::Format
        {
        public:
            typedef ppbox::avformat::CodecInfo CodecInfo;

        public:
            RawFormat()
                : time_scale_(0)
                , video_time_scale_(0)
                , audio_time_scale_(0)
            {
            }

            ~RawFormat()
            {
                if (stun_)
                    delete stun_;
            }

        public:
            void open()
            {
                boost::system::error_code ec;
                stun_ = ppbox::avformat::FormatFactory::create(real_format_, ec);
            }

        private:
            virtual CodecInfo const * codec_from_codec(
                boost::uint32_t category, 
                boost::uint32_t codec_type, 
                boost::system::error_code & ec)
            {
                codec_.category = category;
                codec_.stream_type = 0;
                codec_.codec_type = codec_type;
                codec_.codec_format = 0;
                if (stun_) {
                    CodecInfo const * codec = stun_->codec_from_codec(category, codec_type, ec);
                    if (codec) {
                        codec_ = *codec;
                    }
                    ec.clear();
                }
                codec_.time_scale = time_scale(category);
                return &codec_;
            }

            virtual bool finish_from_codec(
                StreamInfo & info, 
                boost::system::error_code & ec)
            {
                if (stun_) {
                    stun_->finish_from_codec(info, ec);
                }
                info.time_scale = time_scale(info.type);
                return true;
            }

        private:
            boost::uint32_t time_scale(
                boost::uint32_t category)
            {
                if (category == StreamType::VIDE)
                    return video_time_scale_ ? video_time_scale_ : time_scale_;
                if (category == StreamType::AUDI)
                    return audio_time_scale_ ? audio_time_scale_ : time_scale_;
                return time_scale_;
            }

        private:
            ppbox::avformat::Format * stun_;
            CodecInfo codec_;

        private:
            friend class RawMuxer;

            std::string real_format_;
            boost::uint32_t time_scale_;
            boost::uint32_t video_time_scale_;
            boost::uint32_t audio_time_scale_;
        };

    } // namespace mux
} // namespace ppbox


#endif // _PPBOX_MUX_RAW_RAW_FORMAT_H_
