#ifndef _PPBOX_MUX_SINK_H_
#define _PPBOX_MUX_SINK_H_

#include <framework/system/LogicError.h>

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

            virtual ~Sink(){}

            virtual boost::system::error_code write( ppbox::demux::Sample&)
            {
                return framework::system::logic_error::not_supported;
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