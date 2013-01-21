// MuxModule.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxModule.h"
#include "ppbox/mux/Version.h"
#include "ppbox/mux/MuxError.h"
#include "ppbox/mux/MuxerTypes.h"

#include <ppbox/demux/DemuxModule.h>
#include <ppbox/common/UrlHelper.h>

#include <boost/bind.hpp>
using namespace boost::system;

#include <algorithm>

namespace ppbox
{
    namespace mux
    {

        MuxModule::MuxModule(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<MuxModule>(daemon, "MuxModule")
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
            ppbox::demux::DemuxerBase * demuxer,
            framework::string::Url const & config, 
            boost::system::error_code & ec)
        {
            std::string format = config.param("format");
            MuxerBase * muxer = MuxerBase::create(format);
            if (muxer == NULL) {
                ec = error::format_not_support;
            } else {
                ppbox::common::apply_config(muxer->config(), config, "mux.");
                muxer->open(demuxer, ec);
            }
            if (muxer) {
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
} // namespace ppbox
