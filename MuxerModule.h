// MuxerModule.h

#ifndef   _PPBOX_MUXER_MODULE_H_
#define   _PPBOX_MUXER_MODULE_H_

#include "ppbox/mux/Muxer.h"
#include "ppbox/mux/MuxerType.h"

#include <ppbox/common/CommonModuleBase.h>
#include <ppbox/demux/DemuxerModule.h>

#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <map>
#include <vector>

namespace ppbox
{
    namespace mux
    {

        struct MuxerInfo;

        class MuxerModule
            : public ppbox::common::CommonModuleBase<MuxerModule>
        {
        public:
            typedef boost::function<void (
                boost::system::error_code const &,
                Muxer *)
            > open_respone_type;

        public:
            MuxerModule(
                util::daemon::Daemon & daemon)
                : ppbox::common::CommonModuleBase<MuxerModule>(daemon, "muxer")
                , demux_mod_(util::daemon::use_module<ppbox::demux::DemuxerModule>(daemon))
            {
                type_map_["ts"] = MuxerType::TS;
                type_map_["flv"] = MuxerType::FLV;
                type_map_["rtp-es"] = MuxerType::RTPES;
                type_map_["rtp-ts"] = MuxerType::RTPTS;
                type_map_["m3u8"] = MuxerType::m3u8;
            }

            ~MuxerModule()
            {
            }

            virtual boost::system::error_code startup()
            {
                boost::system::error_code ec;
                return ec;
            }

            virtual void shutdown()
            {
            }

        public:
            void async_open(
                std::string playlink,
                std::string format,
                size_t & token,
                open_respone_type const & resp);

            Muxer * open(
                std::string playlink,
                std::string format,
                size_t & token,
                boost::system::error_code & ec);

            // For without using demux module
            Muxer * open(
                ppbox::demux::BufferDemuxer * demuxer,
                std::string format,
                size_t & token);

            boost::system::error_code close(
                size_t close_token,
                boost::system::error_code & ec);

        private:
            MuxerInfo * create(
                std::string format,
                boost::system::error_code & ec);

            void destory(
                MuxerInfo * info);

            void open_callback(
                MuxerInfo * info,
                boost::system::error_code const & ec,
                ppbox::demux::PptvDemuxer * demuxer);

        private:
            ppbox::demux::DemuxerModule & demux_mod_;
            std::vector<MuxerInfo *> muxers_;
            std::string format_;
            std::map<std::string, MuxerType::Enum> type_map_;
            boost::mutex mutex_;
        };
    }
}
#endif // _PPBOX_MUXER_MODULE_H_
