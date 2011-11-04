// AdtsAudioTransfer.h

#ifndef   _PPBOX_MUX_ADTS_AUDIO_TRANSFER_H_
#define   _PPBOX_MUX_ADTS_AUDIO_TRANSFER_H_

#include "ppbox/mux/transfer/Transfer.h"

namespace ppbox
{
    namespace mux
    {

        class AdtsAudioTransfer
            : public Transfer
        {
        public:
            AdtsAudioTransfer()
            {
            }

            ~AdtsAudioTransfer()
            {
            }

            virtual void transfer(ppbox::demux::Sample & sample)
            {
                ppbox::demux::MediaInfo const * audio_stream_info = sample.media_info;
                unsigned int sampling_frequency_index = 0;
                unsigned int channel_configuration = 0;
                boost::uint32_t frame_size = util::buffers::buffer_size(sample.data);
                if (audio_stream_info->format_data.size() > 0
                    && (audio_stream_info->format_data.at(0) >> 3) == 5 ) { // AAC SBR
                        sampling_frequency_index = GetSamplingFrequencyIndex(audio_stream_info->audio_format.sample_rate/2);
                        channel_configuration =  audio_stream_info->audio_format.channel_count / 2;
                } else {
                    sampling_frequency_index = GetSamplingFrequencyIndex(audio_stream_info->audio_format.sample_rate);
                    channel_configuration = audio_stream_info->audio_format.channel_count;
                }
                if (audio_stream_info->format_data.size() == 0) {
                    MakeAdtsHeaderWithData(
                        adts_header_,
                        frame_size,
                        sampling_frequency_index,
                        channel_configuration);
                } else {
                    MakeAdtsHeaderWithBuffer(
                        adts_header_,
                        frame_size,
                        &audio_stream_info->format_data.at(0),
                        audio_stream_info->format_data.size());
                }
                sample.data.push_front(boost::asio::buffer(adts_header_, 7));
                sample.size += 7;
            }

        private:
            void MakeAdtsHeaderWithData(
                boost::uint8_t bits[7], 
                boost::uint32_t frame_size,
                boost::uint32_t sampling_frequency_index,
                boost::uint32_t channel_configuration)
            {
                bits[0] = 0xFF;
                bits[1] = 0xF1; // 0xF9 (MPEG2)
                bits[2] = 0x40 | (sampling_frequency_index << 2) | (channel_configuration >> 2);
                bits[3] = ((channel_configuration&0x3)<<6) | ((frame_size+7) >> 11);
                bits[4] = ((frame_size+7) >> 3)&0xFF;
                bits[5] = (((frame_size+7) << 5)&0xFF) | 0x1F;
                bits[6] = 0xFC;
                /*
                 0:  syncword 12 always: '111111111111' 
                 12: ID 1 0: MPEG-4, 1: MPEG-2 
                 13: layer 2 always: '00' 
                 15: protection_absent 1  
                 16: profile 2  
                 18: sampling_frequency_index 4  
                 22: private_bit 1  
                 23: channel_configuration 3  
                 26: original/copy 1  
                 27: home 1  
                 28: emphasis 2 only if ID == 0 
                 
                 ADTS Variable header: these can change from frame to frame 
                 28: copyright_identification_bit 1  
                 29: copyright_identification_start 1  
                 30: aac_frame_length 13 length of the frame including header (in bytes) 
                 43: adts_buffer_fullness 11 0x7FF indicates VBR 
                 54: no_raw_data_blocks_in_frame 2  
                 ADTS Error check 
                 crc_check 16 only if protection_absent == 0 
                 */
            }

            void MakeAdtsHeaderWithBuffer(
                boost::uint8_t bits[7], 
                boost::uint32_t frame_size,
                boost::uint8_t const * extra,
                boost::uint32_t extraLen)
            {
                const boost::uint8_t * p_extra = extra;
                if( extraLen < 2 || !p_extra )
                    return ; /* no data to construct the headers */
                int i_index = ( (p_extra[0] << 1) | (p_extra[1] >> 7) ) & 0x0f;
                int i_profile = (p_extra[0] >> 3) - 1; /* i_profile < 4 */
                if( i_index == 0x0f && extraLen < 5 )
                    return ; /* not enough data */
                int i_channels = (p_extra[i_index == 0x0f ? 4 : 1] >> 3) & 0x0f;
                /* fixed header */
                bits[0] = 0xff;
                bits[1] = 0xf1; /* 0xf0 | 0x00 | 0x00 | 0x01 */
                bits[2] = (i_profile << 6) | ((i_index & 0x0f) << 2) | ((i_channels >> 2) & 0x01) ;
                bits[3] = (i_channels << 6) | (((frame_size + 7)>> 11) & 0x03);
                /* variable header (starts at last 2 bits of 4th byte) */
                int i_fullness = 0x7ff; /* 0x7ff means VBR */
                /* XXX: We should check if it's CBR or VBR, but no known implementation
                * do that, and it's a pain to calculate this field */
                bits[4] = (frame_size + 7) >> 3;
                bits[5] = (((frame_size + 7) & 0x07) << 5) | ((i_fullness >> 6) & 0x1f);
                bits[6] = ((i_fullness & 0x3f) << 2) /* | 0xfc */;
                /*
                0:  syncword 12 always: '111111111111' 
                12: ID 1 0: MPEG-4, 1: MPEG-2 
                13: layer 2 always: '00' 
                15: protection_absent 1  
                16: profile 2  
                18: sampling_frequency_index 4  
                22: private_bit 1  
                23: channel_configuration 3  
                26: original/copy 1  
                27: home 1  
                28: emphasis 2 only if ID == 0 

                ADTS Variable header: these can change from frame to frame 
                28: copyright_identification_bit 1  
                29: copyright_identification_start 1  
                30: aac_frame_length 13 length of the frame including header (in bytes) 
                43: adts_buffer_fullness 11 0x7FF indicates VBR 
                54: no_raw_data_blocks_in_frame 2  
                ADTS Error check 
                crc_check 16 only if protection_absent == 0 
                */
            }

            boost::uint32_t GetSamplingFrequencyIndex(boost::uint32_t sampling_frequency)
            {
                switch (sampling_frequency) {
                    case 96000: return 0;
                    case 88200: return 1;
                    case 64000: return 2;
                    case 48000: return 3;
                    case 44100: return 4;
                    case 32000: return 5;
                    case 24000: return 6;
                    case 22050: return 7;
                    case 16000: return 8;
                    case 12000: return 9;
                    case 11025: return 10;
                    case 8000:  return 11;
                    case 7350:  return 12;
                    default:    return 0;
                }
            }

        private:
            boost::uint8_t adts_header_[7];

        };
    } // namespace mux
} // namespace ppbox
#endif // _PPBOX_MUX_ADTS_AUDIO_TRANSFER_H_
