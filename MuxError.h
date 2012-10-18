// Error.h

#ifndef _PPBOX_MUX_MUX_ERROR_H_
#define _PPBOX_MUX_MUX_ERROR_H_

namespace ppbox
{
    namespace mux
    {

        namespace error {

            enum errors
            {
                mux_success = 0,
                mux_not_open,
                mux_already_open,
                mux_format_error,
                mux_playlink_error,
                mux_stream_count_error,
                mux_set_metadata_error,
                mux_set_audiotag_error,
                mux_set_videotag_error,
                mux_segment_end,
                mux_invalid_sample,
                mux_not_support,
                mux_other_error = 3000, 
            };

            namespace detail {

                class mux_category
                    : public boost::system::error_category
                {
                public:
                    mux_category()
                    {
                        register_category(*this);
                    }

                    const char* name() const
                    {
                        return "mux";
                    }

                    std::string message(int value) const
                    {
                        switch (value) {
                            case mux_not_open:
                                return "mux not open";
                            case mux_already_open:
                                return "mux already open";
                            case mux_format_error:
                                return "mux setting error format";
                            case mux_playlink_error:
                                return "mux setting error playlink";
                            case mux_stream_count_error:
                                return "mux get stream count error";
                            case mux_set_metadata_error:
                                return "mux set metadata error";
                            case mux_set_audiotag_error:
                                return "mux set audio information error";
                            case mux_set_videotag_error:
                                return "mux set video information error";
                            case mux_segment_end:
                                return "mux segment end";
                            case mux_not_support:
                                return "mux not support";
                            case mux_invalid_sample:
                                return "mux invalid sample";
                            default:
                                return "mux other error";
                        }
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::mux_category instance;
                return instance;
            }

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

        } // namespace error

    } // namespace mux
} // namespace ppbox

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<ppbox::mux::error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using ppbox::mux::error::make_error_code;
#endif

    }
}

#endif // _PPBOX_MUX_MUX_ERROR_H_
