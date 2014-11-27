// MuxModule.cpp

#include "just/mux/Common.h"
#include "just/mux/MuxModule.h"
#include "just/mux/Version.h"
#include "just/mux/MuxError.h"
#include "just/mux/MuxerTypes.h"

#include <just/demux/DemuxModule.h>
#include <just/common/UrlHelper.h>

#include <boost/bind.hpp>
using namespace boost::system;

#include <algorithm>

namespace just
{
    namespace mux
    {

        MuxModule::MuxModule(
            util::daemon::Daemon & daemon)
            : just::common::CommonModuleBase<MuxModule>(daemon, "MuxModule")
        {
        }

        MuxModule::~MuxModule()
        {
        }

        error_code MuxModule::startup()
        {
            error_code ec;
            return ec;
        }

        void MuxModule::shutdown()
        {
        }

        MuxerBase * MuxModule::open(
            just::demux::DemuxerBase * demuxer,
            framework::string::Url const & config, 
            boost::system::error_code & ec)
        {
            std::string format = config.param("format");
            MuxerBase * muxer = MuxerFactory::create(io_svc(), format, ec);
            if (muxer) {
                just::common::apply_config(muxer->config(), config, "mux.");
                muxer->open(demuxer, ec);
                boost::mutex::scoped_lock lock(mutex_);
                muxers_.push_back(muxer);
            }
            return muxer;
        }

        bool MuxModule::close(
            MuxerBase * muxer, 
            error_code & ec)
        {
            {
                boost::mutex::scoped_lock lock(mutex_);
                std::vector<MuxerBase *>::iterator iter = 
                    std::find(muxers_.begin(), muxers_.end(), muxer);
                if (iter == muxers_.end()) {
                    ec = framework::system::logic_error::item_not_exist;
                    muxer = NULL;
                } else {
                    muxers_.erase(iter);
                }
            }
            if (muxer) {
                muxer->close(ec);
                delete muxer;
            }
            return !ec;
        }

    } // namespace mux
} // namespace just
