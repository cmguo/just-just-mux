// MuxModule.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxModule.h"
#include "ppbox/mux/Version.h"
#include "ppbox/mux/MuxerType.h"

#include <ppbox/demux/DemuxModule.h>
using namespace ppbox::demux;

#include <boost/bind.hpp>
using namespace boost::system;

#include <algorithm>

namespace ppbox
{
    namespace mux
    {

        struct MuxerInfo
        {
            MuxerInfo(MuxerBase * imuxer)
                : muxer(imuxer)
                , demuxer_id(size_t(-1))
            {
                static size_t static_id = 0;
                id = ++static_id;
            }

            size_t id;
            MuxerBase * muxer;
            size_t demuxer_id;
            BufferDemuxer * demuxer;
            MuxModule::open_respone_type resp;

            struct Finder
            {
                Finder(
                    size_t id)
                    : id_(id)
                {
                }

                bool operator()(
                    MuxerInfo const * info)
                {
                    return info->id == id_;
                }

            private:
                size_t id_;
            };
        };

        MuxModule::MuxModule(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<MuxModule>(daemon, "muxer")
            , demux_mod_(util::daemon::use_module<ppbox::demux::DemuxModule>(daemon))
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

        void MuxModule::async_open(
            std::string playlink,
            std::string format,
            size_t & token,
            open_respone_type const & resp)
        {
            error_code ec;
            MuxerInfo * mux_info = create(format, ec);
            mux_info->resp = resp;
            token = mux_info->id;
            demux_mod_.async_open(
                playlink,
                mux_info->demuxer_id,
                boost::bind(&MuxModule::open_callback, this, mux_info, _1, _2));
        }

        void MuxModule::open_callback(
            MuxerInfo * info,
            error_code const & ec,
            ppbox::demux::BufferDemuxer * demuxer)
        {
            error_code lec = ec;
            if (!lec) {
                info->demuxer = demuxer;
                info->muxer->open(info->demuxer, lec);
            }
            info->resp(lec, info->muxer);
        }

        MuxerBase * MuxModule::open(
            std::string playlink,
            std::string format,
            size_t & token,
            error_code & ec)
        {
            MuxerInfo * mux_info = create(format, ec);
            token = mux_info->id;
            mux_info->demuxer = demux_mod_.open(playlink, mux_info->demuxer_id, ec);
            if (!ec) {
                mux_info->muxer->open(mux_info->demuxer, ec);
                if (!ec) {
                    return mux_info->muxer;
                }
            }
            return NULL;
        }

        MuxerBase * MuxModule::open(
            ppbox::demux::BufferDemuxer * demuxer,
            std::string format,
            size_t & token)
        {
            error_code ec;
            MuxerInfo * mux_info = create(format, ec);
            token = mux_info->id;
            mux_info->demuxer = demuxer;
            mux_info->muxer->open(mux_info->demuxer, ec);
            if (!ec) {
                return mux_info->muxer;
            }
            return NULL;
        }

        error_code MuxModule::close(
            size_t close_token,
            error_code & ec)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::vector<MuxerInfo *>::const_iterator iter = 
                std::find_if(muxers_.begin(), muxers_.end(), MuxerInfo::Finder(close_token));
            if (iter == muxers_.end()) {
                ec = framework::system::logic_error::item_not_exist;
            } else {
                MuxerInfo * muxer_info = *iter;
                if (muxer_info->demuxer_id != size_t(-1)) {
                    lock.unlock();
                    demux_mod_.close(muxer_info->demuxer_id, ec);
                    lock.lock();
                }
                destory(muxer_info);
            }
            return ec;
        }

        MuxerInfo * MuxModule::create(
            std::string format,
            error_code & ec)
        {
            MuxerBase * muxer = MuxerBase::create(format);
            boost::mutex::scoped_lock lock(mutex_);
            MuxerInfo * muxer_info = new MuxerInfo(muxer);
            muxers_.push_back(muxer_info);
            return muxer_info;
        }

        void MuxModule::destory(
            MuxerInfo * info)
        {
            if (info->muxer) {
                info->muxer->close();
                MuxerBase::destory(info->muxer);
                info->muxer = NULL;
            }
            info->demuxer = NULL;
            muxers_.erase(
                std::remove(muxers_.begin(), muxers_.end(), info), 
                muxers_.end());
            delete info;
            info = NULL;
        }

    } // namespace mux
} // namespace ppbox
