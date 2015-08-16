// MuxModule.h

#ifndef _JUST_MUX_MODULE_H_
#define _JUST_MUX_MODULE_H_

#include <framework/string/Url.h>

#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>

namespace just
{
    namespace demux
    {
        class DemuxerBase;
    }

    namespace mux
    {

        class MuxerBase;

        class MuxModule
            : public just::common::CommonModuleBase<MuxModule>
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
            virtual bool startup(
                boost::system::error_code & ec);

            virtual bool shutdown(
                boost::system::error_code & ec);

        public:
            MuxerBase * open(
                just::demux::DemuxerBase * demuxer, 
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
#endif // _JUST_MUX_MODULE_H_
