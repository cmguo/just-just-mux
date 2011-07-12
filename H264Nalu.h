#ifndef      _PPBOX_MUX_H264_NALU_
#define      _PPBOX_MUX_H264_NALU_

#include "ppbox/mux/AvcConfig.h"

namespace ppbox
{
    namespace mux
    {

        class H264Nalu
        {
        public:
            H264Nalu()
            {
            }

            ~H264Nalu()
            {
            }

            static void process_live_video_config(
                boost::uint8_t const * buf,
                boost::uint32_t size,
                Buffer_Array & config_list)
            {
                boost::uint8_t start_code[4];
                start_code[0] = 0x00;
                start_code[1] = 0x00;
                start_code[2] = 0x00;
                start_code[3] = 0x01;

                boost::uint32_t   begin = 0;
                boost::uint32_t   founds = 0;
                for(boost::uint32_t pos = 0; pos < size; pos++) {
                    if ((size - pos) >= 4) {
                        if (memcmp(buf+pos, start_code, 4) == 0) {
                            if (founds > 0) {
                                Item item;
                                item.assign((boost::uint8_t *)(buf+begin), (boost::uint8_t *)(buf+pos));
                                config_list.push_back(item);
                            }
                            founds++;
                            begin = pos + sizeof(start_code);
                        }
                    } else {
                        if (founds > 0) {
                            Item item;
                            item.assign((boost::uint8_t*)(buf+begin), (boost::uint8_t*)(buf+size));
                            config_list.push_back(item);
                        }
                        break;
                    }
                }
            }

            static boost::uint32_t find_start_code_position(
                boost::uint8_t const * buf,
                boost::uint32_t size,
                boost::uint32_t index)
            {
                boost::uint32_t end_pos = 0;
                boost::uint8_t start_code[4];
                start_code[0] = 0x00;
                start_code[1] = 0x00;
                start_code[2] = 0x00;
                start_code[3] = 0x01;

                boost::uint32_t   founds = 0;
                for(boost::uint32_t pos = 0; pos < size; pos++) {
                    if ((size - pos) >= 4) {
                        if (memcmp(buf+pos, start_code, 4) == 0) {
                            founds++;
                            end_pos = pos;
                            if (founds == index) {
                                break;
                            }
                        }
                    } else {
                        break;
                    }
                }
                return end_pos;
            }
        };
    }
}
#endif // End _PPBOX_MUX_H264_NALU_
