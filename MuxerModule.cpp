// MuxerModule.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerModule.h"
#include "ppbox/mux/flv/FlvMux.h"
#include "ppbox/mux/ts/TsMux.h"
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
		  ,ref(2)
            {
                static size_t static_id = 0;
                id = ++static_id;
            }

            size_t id;
            MuxerBase * muxer;
            size_t demuxer_id;
            Demuxer * demuxer;
            MuxerModule::open_respone_type resp;
	    size_t ref;		
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
            MuxerInfo * mux_info = create(format,resp, ec);
            //mux_info->resp = resp;
            token = mux_info->id;
            demux_mod_.async_open(
                playlink,
                mux_info->demuxer_id,
                boost::bind(&MuxerModule::open_callback, this, mux_info, _1, _2));
        }

        void MuxerModule::open_callback(
            MuxerInfo * info,
            error_code const & ec,
            ppbox::demux::Demuxer * demuxer)
        {
	    boost::mutex::scoped_lock lock(mutex_); 
            error_code lec = ec;
            if (!lec) {
                info->demuxer = demuxer;
                info->muxer->open(info->demuxer, lec);
            }
            open_respone_type resp;
            MuxerBase* muxer = NULL;
            resp.swap(info->resp);
	    muxer = info->muxer;
	    if(--info->ref == 0)
              destory(info);
            lock.unlock();
            resp(lec, muxer);
        }

        MuxerBase * MuxerModule::open(
            std::string playlink,
            std::string format,
            size_t & token,
            error_code & ec)
        {
  	    open_respone_type resp;
            MuxerBase* muxer = NULL;
	    MuxerInfo * mux_info = create(format,resp, ec);
            token = mux_info->id;
            mux_info->demuxer = demux_mod_.open(playlink, mux_info->demuxer_id, ec);
	    boost::mutex::scoped_lock lock(mutex_); 
            muxer = mux_info->muxer;
            if (!ec) {
                mux_info->muxer->open(mux_info->demuxer, ec);
            }
            if(--mux_info->ref == 0) destory(mux_info);
            return muxer;
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
                lock.unlock();
                demux_mod_.close(muxer_info->demuxer_id, ec);
                lock.lock();
		if(--muxer_info->ref == 0)
		{
                   destory(muxer_info);
		}
            }
            return ec;
        }

        MuxerInfo * MuxerModule::create(
            std::string format,
	    open_respone_type const & resp,
            error_code & ec)
        {
            MuxerType::Enum muxer_type = MuxerType::NONE;
            std::map<std::string, MuxerType::Enum>::const_iterator iter = 
                type_map_.find(format);
            if (iter != type_map_.end()) {
                muxer_type = iter->second;
            }
            MuxerBase * muxer = NULL;
            switch(muxer_type) {
                case MuxerType::FLV:
                    muxer = new FlvMux;
                    break;
                case MuxerType::TS:
                    muxer = new TsMux;
                    break;
                default:
                    muxer = new FlvMux;
                    break;
            }
            boost::mutex::scoped_lock lock(mutex_);
            MuxerInfo * muxer_info = new MuxerInfo(muxer);
            muxer_info->resp = resp;
            muxers_.push_back(muxer_info);
            return muxer_info;
        }

        void MuxerModule::destory(
            MuxerInfo * info)
        {
	    assert(0 == info->ref);
            if (info->muxer) {
                delete info->muxer;
                info->muxer = NULL;
            }
            info->demuxer = NULL;
            muxers_.erase(
                std::remove(muxers_.begin(), muxers_.end(), info), 
                muxers_.end());
            delete info;
        }
    }
}
