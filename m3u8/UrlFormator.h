// UrlFormator.h

#ifndef _JUST_MUX_M3U8_URL_FORMATOR_H_
#define _JUST_MUX_M3U8_URL_FORMATOR_H_

#include <ostream>

namespace just
{
    namespace mux
    {

        struct String
        {
            String(
                char const * str, 
                size_t len)
                : str(str)
                , len(len)
            {
            }

            char const * str;
            size_t len;
        };

        class UrlFormator
        {
        public:
            UrlFormator(
                std::string const & format)
            {
                parse(format);
            }

        public:
            struct StreamHelper
            {
                StreamHelper(
                    UrlFormator const & formator, 
                    boost::uint64_t values[3])
                    : formator_(formator)
                    , values_(values)
                {
                }

                friend std::ostream & operator<<(
                    std::ostream & os, 
                    StreamHelper const & h)
                {
                    h.formator_.apply(os, h.values_);
                    return os;
                }

            private:
                UrlFormator const & formator_;
                boost::uint64_t * values_;
            };

            StreamHelper const operator()(
                boost::uint64_t values[3]) const
            {
                return StreamHelper(*this, values);
            }

        public:
            void apply(
                std::ostream & out, 
                boost::uint64_t values[3]) const
            {
                for (size_t i = 0; i < strs_.size(); ++i) {
                    if (strs_[i].str) {
                        out.write(strs_[i].str, strs_[i].len);
                    } else {
                        out << values[strs_[i].len];
                    }
                }
            }

        private:
            void parse(
                std::string const & format)
            {
                char const * accept_tokens = "ntd";

                assert(!format.empty());
                char const * p = format.c_str();
                while (*p) {
                    for (char const * q = p; ; ) {
                        if (*q == '%') {
                            if (q > p) {
                                strs_.push_back(String(p, q - p));
                            }
                            ++q;
                            if (*q) {
                                if (char const * t = strchr(accept_tokens, *q)) {
                                    strs_.push_back(String(NULL, t - accept_tokens));
                                } else {
                                    strs_.push_back(String(q, 1));
                                }
                                ++q;
                            }
                            p = q;
                            break;
                        } else if (*q == 0) {
                            strs_.push_back(String(p, q - p));
                            p = q;
                            break;
                        } else {
                            ++q;
                        }
                    }
                }
            }

        private:
            std::vector<String> strs_;
        };

    } // namespace mux
} // namespace just

#endif // _JUST_MUX_M3U8_URL_FORMATOR_H_
