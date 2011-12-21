#ifndef _PPBOX_MUX_SINK_H_
#define _PPBOX_MUX_SINK_H_

#include <framework/system/LogicError.h>

#include <boost/date_time/posix_time/conversion.hpp>

namespace ppbox
{
    namespace demux
    {
        struct Sample;
    }

    namespace mux
    {

        class Sink
        {
        public:
            Sink()
            {
                seq_ = 0;
            }

            virtual ~Sink()
            {
            }

            virtual boost::system::error_code write(
                boost::posix_time::ptime const & time_send, 
                ppbox::demux::Sample & sample)
            {
                return boost::system::error_code();
                //return framework::system::logic_error::not_supported;
            }

            virtual boost::system::error_code on_finish(
                boost::system::error_code const &)
            {
                return boost::system::error_code();
            }

            void attach(size_t n)
            {
                seq_ += n;
            }

            void attach()
            {
                ++seq_;
            }

            bool detach()
            {
                if(--seq_ == 0)
                {
                    delete this;
                    return false;
                }
                return true;
            }

        private:
            int seq_;
        };

    } // namespace rtsp
} // namespace ppbox

#endif 
