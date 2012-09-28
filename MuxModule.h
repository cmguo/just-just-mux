// MuxModule.h

#ifndef _PPBOX_MUX_MODULE_H_
#define _PPBOX_MUXER_MODULE_H_

#include "ppbox/mux/MuxerBase.h"

#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>

namespace ppbox
{
    namespace demux
    {
        class DemuxModule;
    }

    namespace mux
    {

        struct MuxerInfo;

        class MuxModule
            : public ppbox::common::CommonModuleBase<MuxModule>
        {
        public:
            typedef boost::function<void (
                boost::system::error_code const &,
                MuxerBase *)
            > open_respone_type;

        public:
            MuxModule(
                util::daemon::Daemon & daemon);

            ~MuxModule();

            virtual boost::system::error_code startup();

            virtual void shutdown();

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

            // For without using demux module
            MuxerBase * open(
                ppbox::demux::SegmentDemuxer * demuxer,
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
                ppbox::demux::SegmentDemuxer * demuxer);

        private:
            ppbox::demux::DemuxModule & demux_mod_;
            std::vector<MuxerInfo *> muxers_;
            std::string format_;
            boost::mutex mutex_;
        };
    }
}
#endif // _PPBOX_MUX_MODULE_H_
