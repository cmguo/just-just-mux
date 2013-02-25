// AdtsSplitFilter.h

#ifndef _PPBOX_MUX_FILTER_ADTS_SPLIT_FILTER_H_
#define _PPBOX_MUX_FILTER_ADTS_SPLIT_FILTER_H_

#include "ppbox/mux/Filter.h"

#include <framework/system/ScaleTransform.h>

namespace ppbox
{
    namespace mux
    {

        class AdtsSplitFilter
            : public Filter
        {
        public:
            AdtsSplitFilter();

            virtual ~AdtsSplitFilter();

        public:
            virtual bool open(
                MediaInfo const & media_info, 
                std::vector<StreamInfo> const & streams, 
                boost::system::error_code & ec);

            virtual bool get_sample(
                Sample & sample,
                boost::system::error_code & ec);

            virtual void on_seek(
                boost::uint64_t time);

        private:
            boost::uint32_t audio_track_;
            bool is_save_sample_;
            Sample sample_;
            framework::system::ScaleTransform scale_;
            boost::uint32_t sample_per_frame_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_MUX_FILTER_ADTS_SPLIT_FILTER_H_
