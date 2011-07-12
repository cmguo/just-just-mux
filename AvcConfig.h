#ifndef      _PPBOX_MUX_TS_AVCCONFIG_
#define      _PPBOX_MUX_TS_AVCCONFIG_

#include <vector>

namespace ppbox
{
    namespace mux
    {
        typedef std::vector< boost::uint8_t > Item;
        typedef std::vector< Item > Buffer_Array;
        class AvcConfig
        {
        public:
            AvcConfig()
                : buffer_(NULL)
                , size_(0)
            {
            }

            AvcConfig(boost::uint8_t * buf, boost::uint32_t size)
                : buffer_((boost::uint8_t *)malloc(1024))
                , size_(0)
            {
                if (size <= 1024) {
                    memcpy(buffer_, buf, size);
                    size_ = size;
                }
            }

            AvcConfig(boost::uint32_t buffer_size)
                : buffer_((boost::uint8_t *)malloc(buffer_size))
                , size_(buffer_size)
            {
            }

            ~AvcConfig()
            {
                if (buffer_) {
                    free(buffer_);
                    buffer_ = NULL;
                }

            }

            bool creat(void);

            bool creat(
                boost::uint8_t version,
                boost::uint8_t profile,
                boost::uint8_t level,
                boost::uint8_t profile_compatibility,
                boost::uint8_t nalu_lengthSize,
                Buffer_Array   spss,
                Buffer_Array   ppss);

            void set(
                boost::uint8_t version,
                boost::uint8_t profile,
                boost::uint8_t level,
                boost::uint8_t profile_compatibility,
                boost::uint8_t nalu_lengthSize,
                Buffer_Array   spss,
                Buffer_Array   ppss);

            boost::uint8_t configuration_version(void)
            {
                return configuration_version_;
            }

            boost::uint8_t profile(void)
            {
                return profile_;
            }

            boost::uint8_t level(void)
            {
                return level_;
            }

            boost::uint8_t profile_compatibility(void)
            {
                return profile_compatibility_;
            }

            boost::uint8_t nalu_lengthSize(void)
            {
                return nalu_lengthSize_;
            }

            Buffer_Array   sequence_parameters(void)
            {
                return sequence_parameters_;
            }

            Buffer_Array   picture_parameters(void)
            {
                return picture_parameters_;
            }

            boost::uint8_t * data(void);

            boost::uint32_t  data_size(void);

        private:
            boost::uint8_t      * buffer_;
            boost::uint32_t     size_;
            boost::uint8_t      configuration_version_;
            boost::uint8_t      profile_;
            boost::uint8_t      level_;
            boost::uint8_t      profile_compatibility_;
            boost::uint8_t      nalu_lengthSize_;
            Buffer_Array        sequence_parameters_;
            Buffer_Array        picture_parameters_;
        };
    }
}
#endif // End _PPBOX_MUX_TS_AVCCONFIG_
