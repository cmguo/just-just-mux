// MuxerBase.h

#ifndef _PPBOX_MUX_MUXER_BASE_H_
#define _PPBOX_MUX_MUXER_BASE_H_

#include "ppbox/mux/MuxBase.h"

#include <framework/configure/Config.h>

namespace ppbox
{
    namespace mux
    {

        class MuxerBase
        {
        public:
            MuxerBase(
                boost::asio::io_service & io_svc);

            virtual ~MuxerBase();

        public:
            virtual bool open(
                ppbox::demux::DemuxerBase * demuxer, 
                boost::system::error_code & ec) = 0;

            virtual bool setup(
                boost::uint32_t index, 
                boost::system::error_code & ec) = 0;

            virtual bool read(
                Sample & sample,
                boost::system::error_code & ec) = 0;

            virtual bool reset(
                boost::system::error_code & ec) = 0;

            virtual bool time_seek(
                boost::uint64_t & offset,
                boost::system::error_code & ec) = 0;

            virtual bool byte_seek(
                boost::uint64_t & offset,
                boost::system::error_code & ec) = 0;

            virtual boost::uint64_t check_seek(
                boost::system::error_code & ec) = 0;

            virtual bool close(
                boost::system::error_code & ec) = 0;

        public:
            virtual void media_info(
                MediaInfo & info) const = 0;

            virtual void stream_info(
                std::vector<StreamInfo> & streams) const = 0;

            virtual void stream_status(
                StreamStatus & info) const = 0;

        public:
            framework::configure::Config & config()
            {
                return config_;
            }

        private:
            framework::configure::Config config_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MUXER_BASE_H_
