// MuxModule.h

#ifndef _PPBOX_MUX_MODULE_H_
#define _PPBOX_MUX_MODULE_H_

#include <framework/string/Url.h>

#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>

namespace ppbox
{
    namespace demux
    {
        class DemuxerBase;
    }

    namespace mux
    {

        class MuxerBase;

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

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();

        public:
            MuxerBase * open(
                ppbox::demux::DemuxerBase * demuxer, 
                framework::string::Url const & config, 
                boost::system::error_code & ec);

            bool close(
                MuxerBase * muxer, 
                boost::system::error_code & ec);

        private:
            std::vector<MuxerBase *> muxers_;
            boost::mutex mutex_;
        };
    }
}
#endif // _PPBOX_MUX_MODULE_H_
