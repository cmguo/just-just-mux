// FlvMetadata.h
#ifndef      _PPBOX_MUX_FLV_METADATA_
#define      _PPBOX_MUX_FLV_METADATA_

#define FLV_SCRIPTDATAOBJECT    18
namespace ppbox
{
    namespace mux
    {
        typedef boost::uint8_t uint24_t[3];
        typedef boost::int8_t  int24_t[3];

        typedef struct {
            uint24_t               Signature;
            boost::uint8_t         Version;
            boost::uint8_t         Flags;
            boost::uint8_t         DataOffset[4];
        } FlvHeader;

        typedef struct {
            boost::uint8_t      TagType;
            uint24_t            DataSize;
            uint24_t            Timestamp;
            boost::uint8_t      TimeExtand;
            uint24_t            Reserved;
        } FlvTag;

        typedef struct {
            // Attribute: SoundFormat UB[4], SoundRate UB[2], SoundSize UB[1], SoundType[1]
            boost::uint8_t       SoundAttribute;
            boost::uint8_t       AACPacketType;
        } AudioTagHeader;

        typedef struct {
            // Attribute: FrameType UB[4], CodecID UB[4]
            boost::uint8_t       VideoAttribute;
            boost::uint8_t       AVCPacketType;
            int24_t              CompositionTime;
        } VideoTagHeader;

        typedef struct {
            boost::uint32_t       sample_rate;
            boost::uint32_t       channel_count;
            boost::uint32_t       audio_codec;
            boost::uint32_t       sample_size;
        } FlvAudioInfo;

        typedef struct {
            boost::uint32_t        frame_type;
            boost::uint32_t        video_codec;
        } FlvVideoInfo;

        typedef struct {
            boost::int32_t hasKeyframes;
            boost::int32_t hasVideo;
            boost::int32_t hasAudio;
            boost::int32_t hasMetadata;
            boost::int32_t hasCuePoints;
            boost::int32_t canSeekToEnd;

            double audiocodecid;
            double audiosamplerate;
            double audiodatarate;
            double audiosamplesize;
            double audiodelay;
            boost::int32_t stereo;

            double videocodecid;
            double framerate;
            double videodatarate;
            double height;
            double width;

            double datasize;
            double audiosize;
            double videosize;
            double filesize;

            double lasttimestamp;
            double lastvideoframetimestamp;
            double lastkeyframetimestamp;
            double lastkeyframelocation;

            boost::int32_t keyframes;
            double *filepositions;
            double *times;
            double duration;

            char metadatacreator[256];
            char creator[256];

            boost::int32_t onmetadatalength;
            boost::int32_t metadatasize;
            size_t onlastsecondlength;
            size_t lastsecondsize;
            boost::int32_t hasLastSecond;
            boost::int32_t lastsecondTagCount;
            size_t onlastkeyframelength;
            size_t lastkeyframesize;
            boost::int32_t hasLastKeyframe;
        } TagMetaData;

        // template
        struct Buffer
        {
            Buffer()
                : size(4096)
                , buf((boost::uint8_t *)malloc(size))
                , used(0)
            {
            }

            ~Buffer()
            {
                if (buf) {
                    free(buf);
                    used = 0;
                }
                size = 0;
            }
            boost::uint32_t size;
            boost::uint8_t * buf;
            boost::uint32_t used;
        };

        class FlvMetadata
        {
        public:
            FlvMetadata(char const * creator = NULL)
            {
                char *p = (char*)&metadata_;
                memset(p, 0, sizeof(TagMetaData));
                if(creator != NULL) {
                    strncpy(metadata_.creator, creator, sizeof(metadata_.creator));
                } else {
                    strncpy(metadata_.creator, "pptv", sizeof(metadata_.creator));
                }
                strncpy(
                    metadata_.metadatacreator,
                    "PPTV Metadata for FLV stream\0",
                    sizeof(metadata_.metadatacreator));
            }

            TagMetaData & Metadata()
            {
                return metadata_;
            }

            Buffer const & header()
            {
                return header_;
            }

            Buffer const & data()
            {
                return data_;
            }

            void writeFLVMetaData()
            {
                // Data
                // ScriptDataObject
                data_.used = 0;
                data_.used += writeFLVScriptDataObject(data_.buf);
                // onMetaData
                metadata_.onmetadatalength = 16; // 生成16项metadata信息
                data_.used += writeFLVScriptDataECMAArray(data_.buf+data_.used, "onMetaData", metadata_.onmetadatalength);
                // creator
                if(strlen(metadata_.creator) != 0) {
                    data_.used += writeFLVScriptDataValueString(data_.buf+data_.used, "creator", metadata_.creator);
                }
                // metadatacreator
                data_.used += writeFLVScriptDataValueString(data_.buf+data_.used, "metadatacreator", metadata_.metadatacreator);
                // hasKeyframes
                data_.used += writeFLVScriptDataValueBool(data_.buf+data_.used, "hasKeyframes", metadata_.hasKeyframes);
                // hasVideo
                data_.used += writeFLVScriptDataValueBool(data_.buf+data_.used, "hasVideo", metadata_.hasVideo);
                // hasAudio
                data_.used += writeFLVScriptDataValueBool(data_.buf+data_.used, "hasAudio", metadata_.hasAudio);
                // hasMetadata
                data_.used += writeFLVScriptDataValueBool(data_.buf+data_.used, "hasMetadata", metadata_.hasMetadata);
                // canSeekToEnd
                data_.used += writeFLVScriptDataValueBool(data_.buf+data_.used, "canSeekToEnd", metadata_.canSeekToEnd);
                // duration
                data_.used += writeFLVScriptDataValueDouble(data_.buf+data_.used, "duration", metadata_.duration);
                if(metadata_.hasVideo == 1) {
                    // videocodecid
                    data_.used += writeFLVScriptDataValueDouble(data_.buf+data_.used, "videocodecid", metadata_.videocodecid);
                    // width
                    if(metadata_.width != 0.0) {
                        data_.used += writeFLVScriptDataValueDouble(data_.buf+data_.used, "width", metadata_.width);
                    }
                    // height
                    if(metadata_.height != 0.0) {
                        data_.used += writeFLVScriptDataValueDouble(data_.buf+data_.used, "height", metadata_.height);
                    }
                    // framerate
                    data_.used += writeFLVScriptDataValueDouble(data_.buf+data_.used, "framerate", metadata_.framerate);
                }

                if(metadata_.hasAudio == 1) {
                    // audiocodecid
                    data_.used += writeFLVScriptDataValueDouble(data_.buf+data_.used, "audiocodecid", metadata_.audiocodecid);
                    // audiosamplerate
                    data_.used += writeFLVScriptDataValueDouble(data_.buf+data_.used, "audiosamplerate", metadata_.audiosamplerate);
                    // audiosamplesize
                    data_.used += writeFLVScriptDataValueDouble(data_.buf+data_.used, "audiosamplesize", metadata_.audiosamplesize);
                    // stereo
                    data_.used += writeFLVScriptDataValueBool(data_.buf+data_.used, "stereo", metadata_.stereo);
                }

                data_.used += writeFLVScriptDataVariableArrayEnd(data_.buf+data_.used);
                metadata_.metadatasize = data_.used;
                data_.used += writeFLVPreviousTagSize(data_.buf+data_.used, data_.used + sizeof(FlvTag));

                // header
                header_.used = 0;
                FlvTag flvtag;
                char *t = (char *)&flvtag;
                memset(t, 0, sizeof(FlvTag));
                flvtag.TagType = FLV_SCRIPTDATAOBJECT;
                flvtag.DataSize[0] = ((metadata_.metadatasize >> 16) & 0xff);
                flvtag.DataSize[1] = ((metadata_.metadatasize >> 8) & 0xff);
                flvtag.DataSize[2] = (metadata_.metadatasize & 0xff);
                memcpy(header_.buf, t, sizeof(FlvTag));
                header_.used = sizeof(FlvTag);
            }

        private:
            boost::uint32_t writeFLVScriptDataObject(boost::uint8_t * buf)
            {
                char type;
                type = 2;
                memcpy(buf, &type, 1);
                return 1;
            }

            boost::uint32_t writeFLVScriptDataECMAArray(
                boost::uint8_t * buf,
                const char *name,
                boost::uint32_t len)
            {
                boost::uint32_t datasize = 0;
                unsigned char length[4];
                char type;

                datasize += writeFLVScriptDataString(buf+datasize, name);
                type = 8;    // ECMAArray
                memcpy(buf+datasize, &type, 1);
                datasize += 1;

                length[0] = ((len >> 24) & 0xff);
                length[1] = ((len >> 16) & 0xff);
                length[2] = ((len >> 8) & 0xff);
                length[3] = (len & 0xff);
                memcpy(buf+datasize, length, 4);
                datasize += 4;

                return datasize;
            }

            boost::uint32_t writeFLVScriptDataString(
                boost::uint8_t * buf,
                const char *s)
            {
                boost::uint32_t datasize = 0, len;
                unsigned char length[2];
                len = strlen(s);
                if(len > 0xffff) {
                    datasize += writeFLVScriptDataLongString(buf+datasize, s);
                } else {
                    length[0] = ((len >> 8) & 0xff);
                    length[1] = (len & 0xff);
                    memcpy(buf+datasize, length, 2);
                    datasize += 2;
                    memcpy(buf+datasize, s, len);
                    datasize += len;
                }
                return datasize;
            }

            boost::uint32_t writeFLVScriptDataLongString(
                boost::uint8_t * buf,
                const char *s)
            {
                boost::uint32_t datasize = 0, len;
                unsigned char length[4];
                len = strlen(s);
                if(len > 0xffffffff) {
                    len = 0xffffffff;
                }
                length[0] = ((len >> 24) & 0xff);
                length[1] = ((len >> 16) & 0xff);
                length[2] = ((len >> 8) & 0xff);
                length[3] = (len & 0xff);

                memcpy(buf+datasize, length, 4);
                datasize += 4;
                memcpy(buf+datasize, s, len);
                datasize += len;
                return datasize;
            }

            boost::uint32_t writeFLVScriptDataValueString(
                boost::uint8_t * buf,
                const char *name,
                const char *value)
            {
                boost::uint32_t datasize = 0;
                char type;

                if(name != NULL) {
                    datasize += writeFLVScriptDataString(buf+datasize, name);
                }
                type = 2;    // DataString
                memcpy(buf+datasize, &type, 1);
                datasize += 1;
                datasize += writeFLVScriptDataString(buf+datasize, value);

                return datasize;
            }

            boost::uint32_t writeFLVScriptDataValueBool(
                boost::uint8_t * buf,
                const char *name,
                int value)
            {
                size_t datasize = 0;
                char type;

                if(name != NULL) {
                    datasize += writeFLVScriptDataString(buf+datasize, name);
                }

                type = 1;    // Bool
                memcpy(buf+datasize, &type, 1);
                datasize += 1;
                datasize += writeFLVBool(buf+datasize, value);

                return datasize;
            }

            boost::uint32_t writeFLVBool(
                boost::uint8_t * buf,
                int value)
            {
                boost::uint32_t datasize = 0;
                unsigned char b;
                b = (value & 1);
                memcpy(buf+datasize, &b, 1);
                datasize += 1;
                return datasize;
            }

            boost::uint32_t writeFLVScriptDataValueDouble(
                boost::uint8_t * buf,
                const char *name,
                double value)
            {
                size_t datasize = 0;
                char type;
                if(name != NULL) {
                    datasize += writeFLVScriptDataString(buf, name);
                }
                type = 0;    // Double
                memcpy(buf+datasize, &type, 1);
                datasize += 1;
                datasize += writeFLVDouble(buf+datasize, value);
                return datasize;
            }

            boost::uint32_t writeFLVDouble(
                boost::uint8_t * buf,
                double value)
            {
                union {
                    unsigned char dc[8];
                    double dd;
                } d;
                unsigned char b[8];
                boost::uint32_t datasize = 0;
                d.dd = value;
                b[0] = d.dc[7];
                b[1] = d.dc[6];
                b[2] = d.dc[5];
                b[3] = d.dc[4];
                b[4] = d.dc[3];
                b[5] = d.dc[2];
                b[6] = d.dc[1];
                b[7] = d.dc[0];
                memcpy(buf+datasize, b, 8);
                datasize += 8;
                return datasize;
            }

            boost::uint32_t writeFLVScriptDataVariableArrayEnd(boost::uint8_t * buf) {
                size_t datasize = 0;
                unsigned char length[3];
                length[0] = 0;
                length[1] = 0;
                length[2] = 9;
                memcpy(buf+datasize, length, 3);
                datasize += 3;
                return datasize;
            }

            boost::uint32_t writeFLVPreviousTagSize(boost::uint8_t * buf, boost::uint32_t datasize) {
                unsigned char length[4];
                // 小头到大头转换
                length[0] = ((datasize >> 24) & 0xff);
                length[1] = ((datasize >> 16) & 0xff);
                length[2] = ((datasize >> 8) & 0xff);
                length[3] = (datasize & 0xff);
                memcpy(buf, length, 4);
                return 4;
            }

        private:
            TagMetaData metadata_;
            Buffer header_;
            Buffer data_;

        };
    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FLV_METADATA_
