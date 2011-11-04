//FlvMuxWriter.h

#ifndef      _PPBOX_MUX_FLV_WRITER_
#define      _PPBOX_MUX_FLV_WRITER_

#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/flv/FlvMetadata.h"
#include "ppbox/mux/ByteOrder.h"
using namespace ppbox::demux;

#include <framework/system/BytesOrder.h>
#include <util/buffers/BufferSize.h>
using namespace framework::system;

#include <boost/cstdint.hpp>
#include <boost/asio/streambuf.hpp>
#include <iostream>

namespace ppbox
{
    namespace mux
    {
        class FlvMuxWriter
        {
        public:
            FlvMuxWriter(MediaFileInfo const & media_info)
              : is_open_(false)
              , max_header_size_(16 * 1024)
              , max_metadata_size_(16 * 1024)
              , header_size_(0)
              , header_((unsigned char *)malloc(max_header_size_))
              , metadata_((unsigned char *)malloc(max_metadata_size_))
              , media_info_(media_info)
              {
                  memset(&flvheader_, 0, sizeof(FlvHeader));
                  flvheader_.Signature[0]  = 'F';
                  flvheader_.Signature[1]  = 'L';
                  flvheader_.Signature[2]  = 'V';
                  flvheader_.Version       = 0x01;
                  flvheader_.Flags         = 0x05;
                  flvheader_.DataOffset[0] = 0x0;
                  flvheader_.DataOffset[1] = 0x0;
                  flvheader_.DataOffset[2] = 0x0;
                  flvheader_.DataOffset[3] = 0x9;
              }

              ~FlvMuxWriter()
              {
                  if (header_) {
                      free(header_);
                      header_ = NULL;
                  }

                  if (metadata_) {
                      free(metadata_);
                      metadata_ = NULL;
                  }
              }

              void open(size_t stream_index, 
                        ppbox::demux::MediaInfo const & stream_info)
              {
                  if (!is_open_) {
                      header_size_ = 0;
                      metadata_size_ = 0;
                      memcpy(metadata_, (unsigned char*)&flvheader_, 9);
                      metadata_size_ += 9;
                      boost::uint32_t PreviousTagSize = 0;
                      memcpy(metadata_ + metadata_size_, (unsigned char*)&PreviousTagSize, 4);
                      metadata_size_ += 4;
                      is_open_ = true;
                  }

                  boost::uint32_t tag_data_len = stream_info.format_data.size();
                  unsigned char const *tag_data = tag_data_len ? &stream_info.format_data.at(0) : NULL;
                  if (stream_info.sub_type == VIDEO_TYPE_AVC1) {
                      if (tag_data) {
                          flvtag_.TagType = TAG_TYPE_VIDEO;
                          VideoTagHeader videotagheader;
                          videotagheader.VideoAttribute = 0x17;
                          videotagheader.AVCPacketType = 0x0;
                          memset(&videotagheader.CompositionTime, 0, sizeof(videotagheader.CompositionTime));
                          write_header_to_buffer(
                              (unsigned char*)&videotagheader, 5,
                              tag_data, tag_data_len, true);
                      }
                  }
                  else if (stream_info.sub_type == AUDIO_TYPE_MP4A) {
                      if (tag_data) {
                          flvtag_.TagType = TAG_TYPE_AUDIO;
                          AudioTagHeader audiotagheader;
                          audiotagheader.SoundAttribute = 0xAF;
                          audiotagheader.AACPacketType = 0x0;
                          write_header_to_buffer(
                              (unsigned char*)&audiotagheader, 2,
                              tag_data, tag_data_len, false);
                      }
                  } else if (stream_info.sub_type == AUDIO_TYPE_WMA2) {
                      if (tag_data) {
                          flvtag_.TagType = TAG_TYPE_AUDIO;
                          AudioTagHeader audiotagheader;
                          audiotagheader.SoundAttribute = 0xDF;
                          audiotagheader.AACPacketType = 0x0;
                          write_header_to_buffer(
                              (unsigned char*)&audiotagheader, 2,
                              tag_data, tag_data_len, false);
                      }
                  }
              }

              void close()
              {
              }

              void HandeVideo(ppbox::demux::Sample const & sample)
              {
                  flvtag_.TagType = TAG_TYPE_VIDEO;
                  VideoTagHeader videotagheader;
                  if(sample.is_sync) {
                      videotagheader.VideoAttribute = 0x17;
                  } else {
                      videotagheader.VideoAttribute = 0x27;
                  }
                  videotagheader.AVCPacketType = 0x01;
                  memset(videotagheader.CompositionTime, 0, sizeof(videotagheader.CompositionTime));
                  write_tag_to_buffer(
                      (unsigned char *)&videotagheader,
                      5,
                      sample,
                      true);
              }

              void HandeAudio(ppbox::demux::Sample const & sample)
              {
                  ppbox::demux::MediaInfo const & audio_stream_info = media_info_.stream_infos[media_info_.audio_index];
                  flvtag_.TagType = TAG_TYPE_AUDIO;
                  boost::uint8_t audio_attribute = 0xFF;
                  switch(audio_stream_info.sub_type)
                  {
                  case AUDIO_TYPE_MP4A:
                      audio_attribute = audio_attribute&0xAF;
                      break;
                  case AUDIO_TYPE_MP1A:
                      audio_attribute = audio_attribute&0x2F;
                      break;
                  case AUDIO_TYPE_WMA2:
                      audio_attribute = audio_attribute&0xDF;
                      break;
                  default:
                      audio_attribute = audio_attribute&0xAF;
                      break;
                  }

                  if (audio_stream_info.audio_format.sample_rate >= 44100 ) {
                      audio_attribute = audio_attribute&0xFF;
                  } else if (audio_stream_info.audio_format.sample_rate >= 24000 ){
                      audio_attribute = audio_attribute&0xFB;
                  } else if (audio_stream_info.audio_format.sample_rate >= 12000) {
                      audio_attribute = audio_attribute&0xF7;
                  } else if (audio_stream_info.audio_format.sample_rate >= 6000) {
                      audio_attribute = audio_attribute&0xF3;
                  } else {
                      audio_attribute = audio_attribute&0xFF;
                  }

                  if (audio_stream_info.audio_format.channel_count <= 1) {
                      // for aac always 1;
                      audio_attribute = audio_attribute&0xFF;
                  } else {
                      audio_attribute = audio_attribute&0xFF;
                  }

                  if (8 == audio_stream_info.audio_format.sample_size) {
                      audio_attribute = audio_attribute&0xFE;
                  } else {
                      audio_attribute = audio_attribute&0xFF;
                  }
                  AudioTagHeader audiotagheader;
                  audiotagheader.SoundAttribute = audio_attribute;
                  audiotagheader.AACPacketType = 0x01;

                  write_tag_to_buffer(
                      (unsigned char*)&audiotagheader,
                      2,
                      sample,
                      false);
              }

              boost::asio::const_buffer head_buffer(void) const
              {
                  return boost::asio::buffer(header_, header_size_);
              }

              boost::asio::const_buffer metadata_buffer(void) const
              {
                  return boost::asio::buffer(metadata_, metadata_size_);
              }

              std::vector<boost::asio::const_buffer> const & get_tag(void) const
              {
                  return tag_data_;
              }

              void handle_metadata()
              {
                  metadata_tag_.Metadata().duration = 1000;
                  metadata_tag_.Metadata().hasAudio = 1;
                  metadata_tag_.Metadata().hasVideo = 1;
                  metadata_tag_.Metadata().hasKeyframes = 1;
                  metadata_tag_.Metadata().hasMetadata = 1;
                  ppbox::demux::MediaInfo const & video_info = media_info_.stream_infos[media_info_.video_index];
                  ppbox::demux::MediaInfo const & audio_info = media_info_.stream_infos[media_info_.audio_index];
                  if (video_info.sub_type == VIDEO_TYPE_AVC1) {
                      metadata_tag_.Metadata().videocodecid = 7.0;
                  } else {
                      metadata_tag_.Metadata().videocodecid = 7.0;
                  }
                  metadata_tag_.Metadata().height = video_info.video_format.height;
                  metadata_tag_.Metadata().width = video_info.video_format.width;
                  metadata_tag_.Metadata().framerate = video_info.video_format.frame_rate;
                  // Audio
                  if (audio_info.sub_type == AUDIO_TYPE_MP4A) {
                      metadata_tag_.Metadata().audiocodecid = 10.0;
                  } else if (audio_info.sub_type == AUDIO_TYPE_MP1A){
                      metadata_tag_.Metadata().audiocodecid = 2.0;
                  } else {
                      metadata_tag_.Metadata().audiocodecid = 10.0;
                  }
                  metadata_tag_.Metadata().audiosamplerate = audio_info.audio_format.sample_rate;
                  metadata_tag_.Metadata().audiosamplesize = audio_info.audio_format.sample_size;
                  if (audio_info.audio_format.channel_count) {
                      metadata_tag_.Metadata().stereo = 1;
                  } else {
                      metadata_tag_.Metadata().stereo = 0;
                  }
                  metadata_tag_.writeFLVMetaData();
                  memcpy(metadata_+metadata_size_, metadata_tag_.header().buf, metadata_tag_.header().used);
                  metadata_size_ += metadata_tag_.header().used;
                  memcpy(metadata_+metadata_size_, metadata_tag_.data().buf, metadata_tag_.data().used);
                  metadata_size_ += metadata_tag_.data().used;
              }

              FlvMetadata & Metadata()
              {
                  return metadata_tag_;
              }

        private:
            void setTagSizeAndTimestamp(boost::uint32_t size, boost::uint32_t timestamp)
            {
                host_uint24_to_big_endian(size, flvtag_.DataSize);
                host_uint24_to_big_endian(timestamp, flvtag_.Timestamp);
                flvtag_.TimeExtand   = 0x00;
                flvtag_.Reserved[0]  = 0x00;
                flvtag_.Reserved[1]  = 0x00;
                flvtag_.Reserved[2]  = 0x00;
            }

            void resetTagSzie(boost::uint32_t size)
            {
                host_uint24_to_big_endian(size, flvtag_.DataSize);
            }

            void write_tag_to_buffer(
                unsigned char const * sub_tag,
                boost::uint32_t sub_tag_len,
                ppbox::demux::Sample const & sample,
                bool video)
            {
                boost::uint32_t tag_total_len = 0;
                tag_total_len = util::buffers::buffer_size(sample.data);
                if (video) {
                    setTagSizeAndTimestamp(
                        tag_total_len+5,
                        sample.time);
                } else {
                    setTagSizeAndTimestamp(
                        tag_total_len+2,
                        sample.time);
                }
                tag_total_len = tag_total_len+sub_tag_len+TAG_LENGTH;
                boost::uint32_t tag_head_length = 0;
                memcpy(tag_header_buffer_, (unsigned char *)&flvtag_, TAG_LENGTH);
                tag_head_length += TAG_LENGTH;
                memcpy(tag_header_buffer_+tag_head_length, sub_tag, sub_tag_len);
                tag_head_length += sub_tag_len;

                tag_data_.clear();
                tag_data_.push_back(boost::asio::buffer(tag_header_buffer_, tag_head_length));
                tag_data_.insert(tag_data_.end(), sample.data.begin(), sample.data.end());
                boost::uint32_t PreviousTagSize = BytesOrder::host_to_big_endian_long(tag_total_len);
                memcpy(tag_size_buffer_, (unsigned char const *)&PreviousTagSize, 4);
                tag_data_.push_back(boost::asio::buffer(tag_size_buffer_, 4));
            }

            void write_header_to_buffer(
                                        unsigned char const * sub_tag,
                                        boost::uint32_t sub_tag_len,
                                        unsigned char const * data_buf,
                                        boost::uint32_t data_buf_len,
                                        bool video
                                        )
            {
                if (video) {
                    setTagSizeAndTimestamp(data_buf_len + 5, 0);
                } else {
                    setTagSizeAndTimestamp(data_buf_len + 2, 0);
                }
                copy_tag_head(sub_tag, sub_tag_len, data_buf_len);
                memcpy(header_ + header_size_, data_buf, data_buf_len);
                header_size_ += data_buf_len;

                boost::uint32_t tag_total_len = data_buf_len + sub_tag_len + TAG_LENGTH;
                boost::uint32_t PreviousTagSize = BytesOrder::host_to_big_endian_long(tag_total_len);
                memcpy(header_ + header_size_, (unsigned char const *)&PreviousTagSize, 4);
                header_size_ += 4;
            }

            void copy_tag_head(
                unsigned char const * sub_tag,
                boost::uint32_t sub_tag_len,
                boost::uint32_t data_buf_len)
            {
                memcpy(header_ + header_size_, (unsigned char *)&flvtag_, TAG_LENGTH);
                header_size_ += TAG_LENGTH;
                memcpy(header_ + header_size_, sub_tag, sub_tag_len);
                header_size_ += sub_tag_len;
                boost::uint32_t add_bytes = 256;
                if ((header_size_ + data_buf_len + add_bytes) > max_header_size_) {
                    header_ = (unsigned char *)realloc(header_, header_size_ + data_buf_len + add_bytes);
                    max_header_size_ = header_size_ + data_buf_len + add_bytes;
                }
            }

        private:
            FlvTag flvtag_;
            FlvHeader flvheader_;
            FlvAudioInfo audio_info_;
            FlvVideoInfo video_info_;
            bool is_open_;
            boost::uint32_t max_header_size_;
            boost::uint32_t max_metadata_size_;

            boost::uint32_t header_size_;
            unsigned char * header_;
            boost::uint32_t metadata_size_;
            unsigned char * metadata_;
            std::vector<boost::asio::const_buffer> tag_data_;
            unsigned char tag_header_buffer_[32];
            unsigned char tag_size_buffer_[4];
            // Metadata Handle 
            FlvMetadata metadata_tag_;
            MediaFileInfo const & media_info_;

        public:
            static boost::uint8_t const TAG_TYPE_AUDIO = 8;
            static boost::uint8_t const TAG_TYPE_VIDEO = 9;
            static boost::uint8_t const TAG_LENGTH = 11;
            static boost::uint8_t const START_CODE_SIZE = 4;
        };

    } // namespace mux

} // namespace ppbox

#endif
