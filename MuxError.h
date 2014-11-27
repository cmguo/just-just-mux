// Error.h

#ifndef _JUST_MUX_MUX_ERROR_H_
#define _JUST_MUX_MUX_ERROR_H_

#include <just/avformat/Error.h>

namespace just
{
    namespace mux
    {

        namespace error {

            enum errors
            {
                not_open = 1, 
                already_open, 
                not_support
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
                            case not_open:
                                return "mux: not open";
                            case already_open:
                                return "mux: already open";
                            case not_support:
                                return "mux: not support";
                            default:
                                return "mux: unknown error";
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
} // namespace just

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<just::mux::error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using just::mux::error::make_error_code;
#endif

    }
}

#endif // _JUST_MUX_MUX_ERROR_H_
