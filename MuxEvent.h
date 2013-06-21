// MuxEvent.h

#ifndef _PPBOX_MUX_MUX_EVENT_H_
#define _PPBOX_MUX_MUX_EVENT_H_

namespace ppbox
{
    namespace mux
    {

        struct MuxEvent
        {
            enum EventType
            {
                begin, 		// the first event, it's safe to ignore it
                discontinuity, 
                end, 
                reset, 
                begin_reset, 
                // if care about time, handle following events
                begin_seek, // begin_seek(time), already followed by a reset event
                finish_seek, // finish_seek(time)
            } type;
            boost::uint32_t itrack;
            boost::uint64_t time;

            MuxEvent(
                EventType type, 
                boost::uint32_t itrack, 
                boost::uint64_t time = 0)
                : type(type)
                , itrack(itrack)
                , time(time)
            {
            }
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_MUX_EVENT_H_
