// MuxerBase.cpp

#include "just/mux/Common.h"
#include "just/mux/MuxerBase.h"

#include <util/daemon/Daemon.h>

namespace just
{
    namespace mux
    {


        MuxerBase::MuxerBase(
            boost::asio::io_service & io_svc)
            : config_(util::daemon::Daemon::from_io_svc(io_svc).config(), "just.mux")
        {
        }

        MuxerBase::~MuxerBase()
        {
        }

    } // namespace mux
} // namespace just
