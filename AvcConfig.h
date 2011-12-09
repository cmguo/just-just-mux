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

            AvcConfig(boost::uint8_t const * buf, boost::uint32_t size)
                : buffer_((boost::uint8_t *)malloc(size))
                , size_(0)
            {
                memcpy(buffer_, buf, size);
                size_ = size;
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

            void set_buffer(boost::uint8_t const * buffer, boost::uint32_t size)
            {
                buffer_ = (boost::uint8_t *)malloc(size);
                memcpy(buffer_, buffer, size);
                size_ = size;
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

            boost::uint8_t configuration_version(void) const
            {
                return configuration_version_;
            }

            boost::uint8_t profile(void) const
            {
                return profile_;
            }

            boost::uint8_t level(void) const
            {
                return level_;
            }

            boost::uint8_t profile_compatibility(void) const
            {
                return profile_compatibility_;
            }

            boost::uint8_t nalu_lengthSize(void) const
            {
                return nalu_lengthSize_;
            }

            Buffer_Array const & sequence_parameters(void) const
            {
                return sequence_parameters_;
            }

            Buffer_Array const & picture_parameters(void) const
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
