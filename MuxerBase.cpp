// MuxerBase.cpp

#include "ppbox/mux/Common.h"
#include "ppbox/mux/MuxerBase.h"

#include <util/daemon/Daemon.h>

namespace ppbox
{
    namespace mux
    {


        MuxerBase::MuxerBase(
            boost::asio::io_service & io_svc)
            : config_(util::daemon::Daemon::from_io_svc(io_svc).config(), "ppbox.mux")
        {
        }

        MuxerBase::~MuxerBase()
        {
        }

    } // namespace mux
} // namespace ppbox
