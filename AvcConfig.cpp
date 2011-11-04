#include "ppbox/mux/Common.h"
#include "ppbox/mux/AvcConfig.h"

//#include <framework/system/BytesOrder.h>
//using namespace framework::system;

#include <vector>

namespace ppbox
{
    namespace mux
    {
        static boost::uint16_t uint16BytesToUInt16BE(boost::uint8_t *src)
        {
            boost::uint8_t buffer[2];
            memcpy(buffer, src, 2);
            return (((boost::uint16_t)buffer[0])<<8) | (((boost::uint16_t)buffer[1]));
        }

        static void UInt16Touint16BytesBE(boost::uint16_t src, boost::uint8_t * obj)
        {
            memcpy(obj, &src, 2);
            boost::uint8_t tmp = *obj;
            *obj = *(obj+ 1);
            *(obj+ 1) = tmp;
        }

        bool AvcConfig::creat(void)
        {
            bool res = true;
            boost::uint32_t position = 0;
            // avc config 的长度至少为6的字节
            if (size_ > 6) {
                configuration_version_ = *(buffer_ + position); ++position;
                profile_               = *(buffer_ + position); ++position;
                level_                 = *(buffer_ + position); ++position;
                profile_compatibility_ = *(buffer_ + position); ++position;
                boost::uint8_t length_size_minus_one;
                length_size_minus_one  = *(buffer_ + position); ++position;
                nalu_lengthSize_       = 1 + (length_size_minus_one&3);

                boost::uint8_t num_seq_params;
                num_seq_params = *(buffer_ + position); ++position;
                num_seq_params &= 31;

                for (unsigned int i=0; i < num_seq_params; i++) {
                    boost::uint16_t param_length;
                    if ((size_ - position) > 2) {
                        param_length = uint16BytesToUInt16BE(buffer_ + position);
                        ++position; ++position;
                        if ((size_ - position) >= param_length) {
                            Item item;
                            boost::uint8_t * begin = buffer_ + position;
                            boost::uint8_t * end   = begin + param_length;
                            item.assign(begin, end);
                            sequence_parameters_.push_back(item);
                            position = position + param_length;
                            begin = NULL;
                            end   = NULL;
                        } else {
                            res = false;
                            break;
                        }
                    } else {
                        res = false;
                        break;
                    }
                }

                if ((size_ - position) > 1 && res != false) {
                    boost::uint8_t num_pic_params = *(buffer_ + position); ++position;
                    for (unsigned int i=0; i < num_pic_params; i++) {
                        boost::uint16_t param_length;
                        if ((size_ - position) > 2) {
                            param_length = uint16BytesToUInt16BE(buffer_ + position);
                            ++position; ++position;
                            if ((size_ - position) >= param_length) {
                                Item item;
                                boost::uint8_t * begin = buffer_ + position;
                                boost::uint8_t * end   = begin + param_length;
                                item.assign(begin, end);
                                picture_parameters_.push_back(item);
                                position = position + param_length;
                                begin = NULL;
                                end   = NULL;
                            } else {
                                res = false;
                                break;
                            }
                        } else {
                            res = false;
                            break;
                        }
                    }
                }
            } else {
                res = false;
            }
            return res;
        }

        bool AvcConfig::creat(
            boost::uint8_t version,
            boost::uint8_t profile,
            boost::uint8_t level,
            boost::uint8_t profile_compatibility,
            boost::uint8_t nalu_lengthSize,
            Buffer_Array   spss,
            Buffer_Array   ppss)
        {
            boost::uint32_t position = 0;
            *(buffer_ + position) = version;   ++position;
            *(buffer_ + position) = profile;   ++position;
            *(buffer_ + position) = level;     ++position;
            *(buffer_ + position) = profile_compatibility;     ++position;
            nalu_lengthSize--;
            *(buffer_ + position) = (nalu_lengthSize | 0xFC);  ++position;
            boost::uint8_t sps_size = (boost::uint8_t)spss.size();
            *(buffer_ + position) = (0xE0 | sps_size);     ++position;
            for(boost::uint32_t i = 0; i < spss.size(); i++) {
                boost::uint16_t param_size = (boost::uint16_t)spss[i].size();
                boost::uint8_t param_size_be[2];
                UInt16Touint16BytesBE(param_size, param_size_be);
                memcpy(buffer_ + position, param_size_be, 2);
                ++position; ++position;
                memcpy(buffer_ + position, &spss[i].at(0), spss[i].size());
                position = position + spss[i].size();
            }

            boost::uint8_t pps_size = (boost::uint8_t)ppss.size();
            *(buffer_ + position) = pps_size;     ++position;
            for(boost::uint32_t i = 0; i < ppss.size(); i++) {
                boost::uint16_t param_size = (boost::uint16_t)ppss[i].size();
                boost::uint8_t param_size_be[2];
                UInt16Touint16BytesBE(param_size, param_size_be);
                memcpy(buffer_ + position, param_size_be, 2);
                ++position; ++position;
                memcpy(buffer_ + position, &ppss[i].at(0), ppss[i].size());
                position = position + ppss[i].size();
            }
            size_ = position;
            return true;
        }

        void AvcConfig::set(
            boost::uint8_t version,
            boost::uint8_t profile,
            boost::uint8_t level,
            boost::uint8_t profile_compatibility,
            boost::uint8_t nalu_lengthSize,
            Buffer_Array   spss,
            Buffer_Array   ppss)
        {
            configuration_version_ = version;
            profile_               = profile;
            level_                 = level;
            profile_compatibility_ = profile_compatibility;
            nalu_lengthSize_       = nalu_lengthSize;
            sequence_parameters_   = spss;
            picture_parameters_    = ppss;
        }

        boost::uint8_t * AvcConfig::data(void)
        {
            return buffer_;
        }

        boost::uint32_t  AvcConfig::data_size(void)
        {
            return size_;
        }
    }
}
