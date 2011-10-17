// MuxerModule.h

#ifndef   _PPBOX_MUXER_MODULE_H_
#define   _PPBOX_MUXER_MODULE_H_

#include "ppbox/mux/MuxerType.h"
#include "ppbox/mux/MuxerBase.h"

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
                MuxerBase *)
            > open_respone_type;

        public:
            MuxerModule(
                util::daemon::Daemon & daemon)
                : ppbox::common::CommonModuleBase<MuxerModule>(daemon, "muxer")
                , demux_mod_(util::daemon::use_module<ppbox::demux::DemuxerModule>(daemon))
            {
                type_map_["ts"] = MuxerType::TS;
                type_map_["flv"] = MuxerType::FLV;
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

            MuxerBase * open(
                std::string playlink,
                std::string format,
                size_t & token,
                boost::system::error_code & ec);

            boost::system::error_code close(
                size_t close_token,
                boost::system::error_code & ec);

        private:
            MuxerInfo * create(
                std::string format,
		open_respone_type const & resp,
                boost::system::error_code & ec);

            void destory(
                MuxerInfo * info);

            void open_callback(
                MuxerInfo * info,
                boost::system::error_code const & ec,
                ppbox::demux::Demuxer * demuxer);

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
