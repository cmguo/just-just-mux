//FlvMuxWriter.h

#ifndef      _PPBOX_MUX_FLV_WRITER_
#define      _PPBOX_MUX_FLV_WRITER_

#include "ppbox/mux/MuxerBase.h"
#include "ppbox/mux/H264Nalu.h"
#include "ppbox/mux/flv/FlvMetadata.h"
#include "ppbox/mux/ByteOrder.h"


#include <framework/memory/MemoryPage.h>
#include <framework/system/BytesOrder.h>
#include <util/buffers/BufferCopy.h>
#include <util/archive/ArchiveBuffer.h>
using namespace framework::system;

#include <boost/cstdint.hpp>
#include <boost/asio/streambuf.hpp>
#include <iostream>

#include <ppbox/demux/DemuxerBase.h>
#include <ppbox/demux/asf/AsfObjectType.h>
using namespace ppbox::demux;

namespace ppbox
{
    namespace mux
    {

        struct FlvFileInfo
        {
            boost::uint32_t duration;
            // video
            std::string video_codec;
            boost::uint32_t frame_rate;
            boost::uint32_t width;
            boost::uint32_t height;
            // audio
            std::string audio_codec;
            boost::uint32_t   channel_count;
            boost::uint32_t   sample_size;
            boost::uint32_t   sample_rate;
        };

        class FlvMuxWriter
        {
        public:
            FlvMuxWriter()
              : is_open_(false)
              , got_first_idr_(false)
              , max_header_size_(16 * 1024)
              , max_metadata_size_(16 * 1024)
              , header_size_(0)
              , header_((unsigned char *)malloc(max_header_size_))
              , metadata_((unsigned char *)malloc(max_metadata_size_))
              , tag_(new MuxTag)
              , media_info_(NULL)
              , key_frame_prefix_size_(0)
              , prefix_size_(0)
              {
                  memset(tag_, 0, sizeof(MuxTag));
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

                  if (tag_) {
                      delete tag_;
                      tag_ = NULL;
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
                      // write PreviousTagSize0
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
                          bool is_live = false;
                          if (stream_info.format_type == MediaInfo::video_avc_byte_stream) {
                              is_live = true;
                          }
                          write_header_to_buffer(
                              (unsigned char*)&videotagheader, 5,
                              tag_data, tag_data_len, true, is_live);
                      }
                  }
                  else if (stream_info.sub_type == AUDIO_TYPE_MP4A) {
                      if (tag_data) {
                          flvtag_.TagType = TAG_TYPE_AUDIO;
                          AudioTagHeader audiotagheader;
                          // codec AAC
                          audiotagheader.SoundAttribute = 0xAF;
                          audiotagheader.AACPacketType = 0x0;
                          if (stream_info.format_type != MediaInfo::audio_microsoft_wave) {
                              write_header_to_buffer(
                                  (unsigned char*)&audiotagheader, 2,
                                  tag_data, tag_data_len, false);
                          } else {
                              boost::uint8_t cbuf[1024];
                              memset(cbuf, 0, sizeof(cbuf));
                              memcpy(cbuf, tag_data, tag_data_len);
                              util::archive::ArchiveBuffer<boost::uint8_t> buf(
                                  cbuf,
                                  sizeof(cbuf), 
                                  tag_data_len);
                              ASF_Audio_Media_Type asf_audio_type;
                              ASFArchive archive(buf);
                              archive >> asf_audio_type;
                              write_header_to_buffer(
                                  (unsigned char*)&audiotagheader, 2,
                                  &asf_audio_type.CodecSpecificData.at(0),
                                  asf_audio_type.CodecSpecificDataSize,
                                  false);
                          }
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
                  } else if (stream_info.sub_type == AUDIO_TYPE_MP1A) {
                      // 支持mp3格式
                      flvtag_.TagType = TAG_TYPE_AUDIO;
                      AudioTagHeader audiotagheader;
                      audiotagheader.SoundAttribute = 0x2F;
                      audiotagheader.AACPacketType = 0x0;
                      write_header_to_buffer(
                          (unsigned char*)&audiotagheader, 2,
                          tag_data, tag_data_len, false);
                  }
              }

              void close()
              {
              }

              void HandeVideo(ppbox::demux::Sample const & sample)
              {
                  flvtag_.TagType = TAG_TYPE_VIDEO;
                  VideoTagHeader videotagheader;
                  // always AVC
                  if(sample.is_sync) {
                      got_first_idr_ = true;
                      videotagheader.VideoAttribute = 0x17;
                  } else {
                      videotagheader.VideoAttribute = 0x27;
                  }
                  if (!got_first_idr_) {
                      tag_->tag_data_buffer = NULL;
                      tag_->tag_data_length = 0;
                      tag_->tag_header_buffer = NULL;
                      tag_->tag_header_length = 0;
                      tag_->tag_size_buffer = NULL;
                      tag_->tag_size_length = 0;
                      return;
                  }
                  videotagheader.AVCPacketType = 0x01;
                  memset(videotagheader.CompositionTime, 0, sizeof(videotagheader.CompositionTime));
                  if (media_info_->video_format_type == MediaInfo::video_avc_packet
                      || media_info_->video_format_type == MediaInfo::video_flv_tag) {
                      // vod
                      write_tag_to_buffer(
                          (unsigned char *)&videotagheader,
                          5,
                          sample,
                          true);
                  } else if (media_info_->video_format_type == MediaInfo::video_avc_byte_stream) {
                      if (media_info_->audio_codec == AUDIO_TYPE_MP4A) {
                          // new live
                          write_tag_to_buffer(
                              (unsigned char *)&videotagheader,
                              5,
                              sample,
                              true,
                              true);
                      } else if (media_info_->audio_codec == AUDIO_TYPE_WMA2) {
                          // old live
                          write_tag_to_buffer(
                              (unsigned char *)&videotagheader,
                              5,
                              sample,
                              true);
                      }
                  }
              }

              void HandeAudio(ppbox::demux::Sample const & sample)
              {
                  flvtag_.TagType = TAG_TYPE_AUDIO;
                  //audio codec
                  boost::uint8_t audio_attribute = 0xFF;
                  switch(media_info_->audio_codec)
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

                  if (media_info_->sample_rate >= 44100 ) {
                      audio_attribute = audio_attribute&0xFF;
                  } else if (media_info_->sample_rate >= 24000 ){
                      audio_attribute = audio_attribute&0xFB;
                  } else if (media_info_->sample_rate >= 12000) {
                      audio_attribute = audio_attribute&0xF7;
                  } else if (media_info_->sample_rate >= 6000) {
                      audio_attribute = audio_attribute&0xF3;
                  } else {
                      audio_attribute = audio_attribute&0xFF;
                  }

                  if (media_info_->channel_count <= 1) {
                      // for aac always 1;
                      audio_attribute = audio_attribute&0xFF;
                  } else {
                      audio_attribute = audio_attribute&0xFF;
                  }

                  if (8 == media_info_->sample_size) {
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

              void set_media_info(MediaFileInfo * media_info)
              {
                  media_info_ = media_info;
              }

              boost::uint32_t get_header_size(void) const
              {
                  return header_size_;
              }

              unsigned char const * get_header_buffer() const
              {
                  return header_;
              }

              boost::uint32_t get_metadata_size(void) const
              {
                  return metadata_size_;
              }

              unsigned char const * get_metadata_buffer() const
              {
                  return metadata_;
              }

              const MuxTag * get_tag(void) const
              {
                  return tag_;
              }

              void handle_metadata(FlvFileInfo const & info)
              {
                  metadata_tag_.Metadata().duration = info.duration;
                  metadata_tag_.Metadata().hasAudio = 1;
                  metadata_tag_.Metadata().hasVideo = 1;
                  metadata_tag_.Metadata().hasKeyframes = 1;
                  metadata_tag_.Metadata().hasMetadata = 1;
                  // video
                  if (info.audio_codec == "avc") {
                      metadata_tag_.Metadata().videocodecid = 7.0;
                  } else {
                      metadata_tag_.Metadata().videocodecid = 7.0;
                  }
                  metadata_tag_.Metadata().height = info.height;
                  metadata_tag_.Metadata().width = info.width;
                  metadata_tag_.Metadata().framerate = info.frame_rate;
                  // Audio
                  if (info.audio_codec == "aac") {
                      metadata_tag_.Metadata().audiocodecid = 10.0;
                  } else if (info.audio_codec == "mp3"){
                      metadata_tag_.Metadata().audiocodecid = 2.0;
                  } else {
                      metadata_tag_.Metadata().audiocodecid = 10.0;
                  }
                  
                  metadata_tag_.Metadata().audiosamplerate = info.sample_rate;
                  metadata_tag_.Metadata().audiosamplesize = info.sample_size;
                  if (info.channel_count >= 2) {
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
                bool video,
                bool live = false)
            {
                boost::uint32_t tag_total_len = 0;
                tag_->tag_header_length = 0;
                tag_->tag_size_length = 4;
                if (live) {
                    unsigned char const * frame_data = sample.size ? &sample.data.at(0) : NULL;
                    boost::uint32_t prefix_size = START_CODE_SIZE;
                    if (sample.is_sync) {
                        if (key_frame_prefix_size_ == 0) {
                            key_frame_prefix_size_ = H264Nalu::find_start_code_position(
                                frame_data,
                                sample.size,
                                3);
                        }
                        prefix_size = key_frame_prefix_size_ + START_CODE_SIZE;
                    } else {
                        if (prefix_size_ == 0 || key_frame_prefix_size_ == 0) {
                            prefix_size_ = H264Nalu::find_start_code_position(
                                frame_data,
                                sample.size,
                                1);
                        }
                        prefix_size = prefix_size_ + START_CODE_SIZE;
                    }

                    boost::uint32_t frame_size = sample.size - prefix_size;
                    setTagSizeAndTimestamp(frame_size+9, sample.time);
                    memcpy(tag_header_buffer_, (unsigned char *)&flvtag_, TAG_LENGTH);
                    tag_->tag_header_length += TAG_LENGTH;
                    memcpy(tag_header_buffer_ + tag_->tag_header_length, sub_tag, sub_tag_len);
                    tag_->tag_header_length += sub_tag_len;
                    boost::uint32_t playload_size = BytesOrder::host_to_big_endian_long(frame_size);
                    memcpy(tag_header_buffer_+tag_->tag_header_length, (boost::uint8_t*)&playload_size, 4);
                    tag_->tag_header_length += 4;
                    tag_->tag_header_buffer = tag_header_buffer_;
                    tag_->tag_data_buffer = frame_data + prefix_size;
                    tag_->tag_data_length = frame_size;

                    tag_total_len = frame_size + sub_tag_len + TAG_LENGTH + START_CODE_SIZE;
                } else {
                    // vod
                    tag_->tag_data_buffer = sample.size ? &sample.data.at(0) : NULL;
                    tag_->tag_data_length = sample.size;
                    tag_total_len = sample.size + sub_tag_len + TAG_LENGTH;
                    if (video) {
                        setTagSizeAndTimestamp(sample.size + 5, sample.time);
                    } else {
                        setTagSizeAndTimestamp(sample.size + 2, sample.time);
                    }

                    memcpy(tag_header_buffer_, (unsigned char *)&flvtag_, TAG_LENGTH);
                    tag_->tag_header_length += TAG_LENGTH;
                    memcpy(tag_header_buffer_ + tag_->tag_header_length, sub_tag, sub_tag_len);
                    tag_->tag_header_length += sub_tag_len;
                    tag_->tag_header_buffer = tag_header_buffer_;
                }

                boost::uint32_t PreviousTagSize = BytesOrder::host_to_big_endian_long(tag_total_len);
                memcpy(tag_size_buffer_, (unsigned char const *)&PreviousTagSize, 4);
                tag_->tag_size_buffer = tag_size_buffer_;
            }

            void write_header_to_buffer(
                                        unsigned char const * sub_tag,
                                        boost::uint32_t sub_tag_len,
                                        unsigned char const * data_buf,
                                        boost::uint32_t data_buf_len,
                                        bool video,
                                        bool live = false
                                        )
            {
                if (live) {
                    Buffer_Array config_list;
                    H264Nalu::process_live_video_config(data_buf, data_buf_len, config_list);
                    // 分配足够保存video avc config信息
                    AvcConfig avc_config((boost::uint32_t)framework::memory::MemoryPage::align_page(data_buf_len * 2));
                    if (config_list.size() >= 2) {
                        Buffer_Array spss;
                        Buffer_Array ppss;
                        spss.push_back(config_list[config_list.size()-2]);
                        ppss.push_back(config_list[config_list.size()-1]);
                        // 设置avc config
                        avc_config.creat(0x01, 0x64, 0x00, 0x15, 0x04, spss, ppss);
                        boost::uint32_t data_size = avc_config.data_size();
                        if (video) {
                            setTagSizeAndTimestamp(data_size + 5, 0);
                        } else {
                            setTagSizeAndTimestamp(data_size + 2, 0);
                        }
                        copy_tag_head(sub_tag, sub_tag_len, data_size);
                        memcpy(header_ + header_size_, avc_config.data(), data_size);
                        header_size_ += data_size;
                        data_buf_len = data_size;
                    }
                } else {
                    if (video) {
                        setTagSizeAndTimestamp(data_buf_len + 5, 0);
                    } else {
                        setTagSizeAndTimestamp(data_buf_len + 2, 0);
                    }
                    copy_tag_head(sub_tag, sub_tag_len, data_buf_len);
                    memcpy(header_ + header_size_, data_buf, data_buf_len);
                    header_size_ += data_buf_len;
                }

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
            bool got_first_idr_;
            boost::uint32_t max_header_size_;
            boost::uint32_t max_metadata_size_;

            boost::uint32_t header_size_;
            unsigned char * header_;
            boost::uint32_t metadata_size_;
            unsigned char * metadata_;
            MuxTag        * tag_;
            unsigned char tag_header_buffer_[32];
            unsigned char tag_size_buffer_[4];
            // Metadata Handle
            FlvMetadata metadata_tag_;
            MediaFileInfo * media_info_;
            boost::uint32_t key_frame_prefix_size_;
            boost::uint32_t prefix_size_;

        public:
            static boost::uint8_t const TAG_TYPE_AUDIO = 8;
            static boost::uint8_t const TAG_TYPE_VIDEO = 9;
            static boost::uint8_t const TAG_LENGTH = 11;
            static boost::uint8_t const START_CODE_SIZE = 4;
        };

    } // namespace mux

} // namespace ppbox

#endif
