// MuxPlayer.h

#ifndef _PPBOX_MUX_PLAYER_H_
#define _PPBOX_MUX_PLAYER_H_

#include <ppbox/common/Dispatcher.h>

namespace ppbox
{
    namespace demux
    {
        class BufferDemuxer;
    }
    namespace mux
    {  
        class Muxer;

        class MuxPlayer : public ppbox::common::PlayInterface
        {
        public:

            MuxPlayer();

            void set(ppbox::mux::Muxer *muxer,
                ppbox::common::session_callback_respone const &resp,
                ppbox::common::Session* session = NULL,
                ppbox::demux::BufferDemuxer* demuxer = NULL);

            virtual ~MuxPlayer();
            virtual boost::system::error_code doing();

        private:
            boost::system::error_code buffering();
            boost::system::error_code playing();
    
        private:
            ppbox::mux::Muxer *muxer_;
            ppbox::demux::BufferDemuxer* demuxer_;
            ppbox::common::Session* session_;
            ppbox::common::session_callback_respone resp_;
        };
    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_PLAYER_H_
