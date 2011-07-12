// Muxer.h
#ifndef _PPBOX_MUX_MUXER_H_
#define _PPBOX_MUX_MUXER_H_

namespace ppbox
{
    namespace demux
    {
        class Demuxer;
    }

    namespace demux
    {
        struct Sample;
    }

    namespace mux
    {
        // For interface
        struct MuxTag
        {
            boost::uint32_t        tag_header_length;          // 适配容器数据幀header的大小
            unsigned char const *  tag_header_buffer;          // 适配容器数据幀header的buffer
            boost::uint32_t        tag_data_length;            // 适配容器数据幀data的大小
            unsigned char const *  tag_data_buffer;            // 适配容器数据幀data的buffer
            boost::uint32_t        tag_size_length;            // 适配容器数据幀size的大小
            unsigned char const *  tag_size_buffer;            // 适配容器数据幀size的buffer

            MuxTag & operator=(MuxTag const & tag)
            {
                tag_header_length  = tag.tag_header_length;
                tag_header_buffer  = tag.tag_header_buffer;
                tag_data_length    = tag.tag_data_length;
                tag_data_buffer    = tag.tag_data_buffer;
                tag_size_length    = tag.tag_size_length;
                tag_size_buffer    = tag.tag_size_buffer;
                return *this;
            }
        };

        struct MuxTagEx
        {
            boost::uint32_t itrack;
            boost::uint32_t time;   // 毫秒
            boost::uint32_t idesc;
            bool is_sync;
            boost::uint32_t        tag_header_length;          // 适配容器数据幀header的大小
            unsigned char const *  tag_header_buffer;          // 适配容器数据幀header的buffer
            boost::uint32_t        tag_data_length;            // 适配容器数据幀data的大小
            unsigned char const *  tag_data_buffer;            // 适配容器数据幀data的buffer
            boost::uint32_t        tag_size_length;            // 适配容器数据幀size的大小
            unsigned char const *  tag_size_buffer;            // 适配容器数据幀size的buffer

            MuxTagEx & operator=(MuxTagEx const & tag)
            {
                itrack             = tag.itrack;
                time               = tag.time;
                idesc              = tag.idesc;
                is_sync            = tag.is_sync;
                tag_header_length  = tag.tag_header_length;
                tag_header_buffer  = tag.tag_header_buffer;
                tag_data_length    = tag.tag_data_length;
                tag_data_buffer    = tag.tag_data_buffer;
                tag_size_length    = tag.tag_size_length;
                tag_size_buffer    = tag.tag_size_buffer;
                return *this;
            }
        };

        struct MediaFileInfo
        {
            MediaFileInfo()
                : duration(0)
                , frame_rate(0)
                , width(0)
                , height(0)
                , channel_count(0)
                , sample_size(0)
                , sample_rate(0)
            {
            }

            void init()
            {
                duration = 0;
                frame_rate = 0;
                width = 0;
                height = 0;
                channel_count = 0;
                sample_size = 0;
                sample_rate = 0;
            }

            std::string     format;
            boost::uint32_t duration;
            // video
            boost::uint32_t   video_codec;
            boost::uint32_t   video_format_type;
            boost::uint32_t   frame_rate;
            boost::uint32_t   width;
            boost::uint32_t   height;
            // audio
            boost::uint32_t   audio_codec;
            boost::uint32_t   audio_format_type;
            boost::uint32_t   channel_count;
            boost::uint32_t   sample_size;
            boost::uint32_t   sample_rate;
        };

        class MuxerBase
        {
        public:
            MuxerBase()
            {
            }

            virtual ~MuxerBase()
            {
            }

        public:
            virtual boost::system::error_code open(
                demux::Demuxer * demuxer, boost::system::error_code & ec) = 0;

            virtual boost::system::error_code read(
                MuxTag * tag,
                boost::system::error_code & ec) = 0;

            virtual boost::system::error_code seek(
                boost::uint32_t time,
                boost::system::error_code & ec) = 0;

            virtual boost::system::error_code pause(
                boost::system::error_code & ec) = 0;

            virtual boost::system::error_code resume(
                boost::system::error_code & ec) = 0;

            virtual void close(void) = 0;

            virtual void reset(void) = 0;

            virtual ppbox::demux::Sample & get_sample(void) = 0;

            virtual unsigned char const * get_head(boost::uint32_t & size) = 0;

            virtual MediaFileInfo const & get_media_info(void) const = 0;

            virtual boost::uint64_t get_current_time(void) = 0;

            virtual boost::uint32_t video_track_index(void) = 0;

            virtual boost::uint32_t audio_track_index(void) = 0;
        };
    }
}

#endif // _PPBOX_MUX_MUXER_H_
