// MmsMuxer.h

#ifndef _PPBOX_MUX_MMS_MMS_MUXER_H_
#define _PPBOX_MUX_MMS_MMS_MUXER_H_

#include "ppbox/mux/asf/AsfMuxer.h"

namespace ppbox
{
    namespace mux
    {

        class MmsTransfer;

        class MmsMuxer
            : public AsfMuxer
        {
        public:
            MmsMuxer();

            ~MmsMuxer();

        public:
            virtual void media_info(
                MediaInfo & info) const;

        private:
            virtual void add_stream(
                StreamInfo & info, 
                std::vector<Transfer *> & transfers);

            virtual void file_header(
                Sample & sample);

        private:
            MmsTransfer * mms_transfer_;
        };

        PPBOX_REGISTER_MUXER("mms", MmsMuxer);

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MMS_MMS_MUXER_H_
