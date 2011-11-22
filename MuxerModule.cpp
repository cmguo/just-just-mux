// MuxerModule.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerModule.h"
#include "ppbox/mux/flv/FlvMux.h"
#include "ppbox/mux/ts/TsMux.h"
#include "ppbox/mux/ts/M3U8Mux.h"
#include "ppbox/mux/rtp/RtpEsMux.h"
#include "ppbox/mux/rtp/RtpTsMux.h"
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
            MuxerInfo(Muxer * imuxer)
                : muxer(imuxer)
                , demuxer_id(size_t(-1))
            {
                static size_t static_id = 0;
                id = ++static_id;
            }

            size_t id;
            Muxer * muxer;
            size_t demuxer_id;
            PptvDemuxer * demuxer;
            MuxerModule::open_respone_type resp;

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

        void MuxerModule::async_open(
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
                boost::bind(&MuxerModule::open_callback, this, mux_info, _1, _2));
        }

        void MuxerModule::open_callback(
            MuxerInfo * info,
            error_code const & ec,
            ppbox::demux::PptvDemuxer * demuxer)
        {
            error_code lec = ec;
            if (!lec) {
                info->demuxer = demuxer;
                info->muxer->open(info->demuxer, lec);
            }
            info->resp(lec, info->muxer);
        }

        Muxer * MuxerModule::open(
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

        Muxer * MuxerModule::open(
            ppbox::demux::PptvDemuxer * demuxer,
            std::string format,
            size_t & token)
        {
            error_code ec;
            MuxerInfo * mux_info = create(format, ec);
            token = mux_info->id;
            mux_info->demuxer = demuxer;
            mux_info->muxer->open(mux_info->demuxer, ec);
            if (!ec) {
                boost::uint32_t seek_time = 0;
                mux_info->muxer->seek(seek_time, ec);
                return mux_info->muxer;
            }
            return NULL;
        }

        error_code MuxerModule::close(
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

        MuxerInfo * MuxerModule::create(
            std::string format,
            error_code & ec)
        {
            MuxerType::Enum muxer_type = MuxerType::NONE;
            std::map<std::string, MuxerType::Enum>::const_iterator iter = 
                type_map_.find(format);
            if (iter != type_map_.end()) {
                muxer_type = iter->second;
            }
            Muxer * muxer = NULL;
            switch(muxer_type) {
                case MuxerType::FLV:
                    muxer = new FlvMux;
                    break;
                case MuxerType::TS:
                    muxer = new TsMux;
                    break;
                case MuxerType::RTPES:
                    muxer = new RtpEsMux;
                    break;
                case MuxerType::RTPTS:
                    muxer = new RtpTsMux;
                    break;
                case MuxerType::m3u8:
                    muxer = new M3U8Mux;
                    break;
                default:
                    muxer = new FlvMux;
                    break;
            }
            boost::mutex::scoped_lock lock(mutex_);
            MuxerInfo * muxer_info = new MuxerInfo(muxer);
            muxers_.push_back(muxer_info);
            return muxer_info;
        }

        void MuxerModule::destory(
            MuxerInfo * info)
        {
            if (info->muxer) {
                info->muxer->close();
                delete info->muxer;
                info->muxer = NULL;
            }
            info->demuxer = NULL;
            muxers_.erase(
                std::remove(muxers_.begin(), muxers_.end(), info), 
                muxers_.end());
            delete info;
            info = NULL;
        }
    }
}
