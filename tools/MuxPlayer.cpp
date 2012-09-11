// RtspSession.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/tools/MuxPlayer.h"
#include "ppbox/mux/Muxer.h"

#include <ppbox/demux/base/BufferDemuxer.h>

#include <framework/logger/StreamRecord.h>
using namespace framework::system::logic_error;
using namespace framework::logger;

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
using namespace boost::system;

namespace ppbox
{
    namespace mux
    {
        MuxPlayer::MuxPlayer()
            : muxer_(NULL)
            , demuxer_(NULL)
            , session_(NULL)
        {
        }

        MuxPlayer::~MuxPlayer()
        {

        }

        void MuxPlayer::set(ppbox::mux::Muxer *muxer,
            ppbox::common::session_callback_respone const &resp,
            ppbox::common::Session* session,
            ppbox::demux::BufferDemuxer* demuxer )
        {
            muxer_ = muxer;
            resp_ = resp;
            session_ = session;
            demuxer_ = demuxer;
        }

        boost::system::error_code MuxPlayer::doing()
        {
            boost::system::error_code ec;

            assert(exit_);

            exit_ = false;
            playing_ = true;

            ec = (NULL == session_)?buffering():playing();

            exit_ = true;

            resp_(ec);
            return ec;
        }

        boost::system::error_code MuxPlayer::buffering()
        {
            boost::system::error_code ec;
            boost::system::error_code ec1;

            while(exit_)
            {
                ec.clear();

                for (boost::int32_t ii = 0; ii < 10; ++ii)
                {
                    demuxer_->get_buffer_time(ec,ec1);
                    if (ec1 /*== boost::asio::error::would_block*/)
                    {
                        break;
                    }

                }
                boost::this_thread::sleep(boost::posix_time::milliseconds(100));
            }

            return ec;
        }

        boost::system::error_code MuxPlayer::playing()
        {
            boost::system::error_code ec;

            {
                boost::uint32_t seek = session_->playlist_[0].beg_;
                if (seek != boost::uint32_t(-1))
                {
                    muxer_->seek(seek,ec);

                    size_t n = 0;
                    while (ec == boost::asio::error::would_block) 
                    {
                        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
                        ++n;
                        if (n == 5) {
                            muxer_->seek(seek,ec);
                            n = 0;
                        }
                    }

                }
                if(ec)
                    return ec;
            }


            while(!exit_)
            {
                if (!playing_)
                {
                    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
                }
                else
                {
                    ec.clear();
                    ppbox::demux::Sample  tag ;
                    muxer_->read(tag,ec);
                    if(!ec)
                    {
                        do 
                        {
                            size_t isend = 0;
                            isend = session_->sinks_[tag.itrack]->write_some(tag.data,ec);
                            if(ec == boost::asio::error::would_block)
                            {
                                if(isend > 0)
                                {
                                    tag.size = tag.size > isend?tag.size-isend:0;
                                    size_t size = 0;
                                    std::deque<boost::asio::const_buffer>::iterator iter = tag.data.begin();
                                    while(iter != tag.data.end())
                                    {
                                        size_t size_new = boost::asio::buffer_size(*iter);
                                        if(isend < (size+size_new))
                                        {
                                            (*iter) = (*iter) + (isend-size);
                                            break;
                                        }
                                        else if(isend == (size+size_new))
                                        {
                                            tag.data.pop_front();
                                            break;
                                        }
                                        else
                                        {
                                            size += size_new;
                                            tag.data.pop_front();
                                        }
                                        iter = tag.data.begin();
                                    }
                                }
                                boost::this_thread::sleep(boost::posix_time::milliseconds(100));
                            }
                            else
                            {
                                break;
                            }
                            //发数据  would_block
                            //
                        }while (!exit_);

                    }

                    if(ec == boost::asio::error::would_block)
                    {//读数据 would_block
                        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
                    }
                    else
                    {
                        break;
                    }
                }
            }

            return ec;
        }

    } // namespace mux
} // namespace ppbox
