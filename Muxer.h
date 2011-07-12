// Muxer.h
namespace ppbox
{
    namespace demux
    {
        class Demuxer;
    }

    namespace mux
    {
        template <typename handler>
        class Muxer
        {
        public:
            Muxer();
            ~Muxer();

            boost::system::error_code open(demux::Demuxer * demuxer)
            {
                boost::system::error_code ec;
                return mux_handle.open(demuxer, ec);
            }

            boost::system::error_code read(MuxTag * tag)
            {
                boost::system::error_code ec;
                return mux_handle.read(tag, ec);
            }

            boost::system::error_code seek(boost::uint32_t time)
            {
                boost::system::error_code ec;
                return mux_handle.seek(time, ec);
            }

            boost::system::error_code pasue()
            {
                boost::system::error_code ec;
                return mux_handle.pasue(ec);
            }

            boost::system::error_code resume()
            {
                boost::system::error_code ec;
                return mux_handle.resume(ec);
            }

            void close(void)
            {
                mux_handle.close();
            }

            void reset(void)
            {
                mux_handle.reset();
            }

            ppbox::demux::Sample & get_sample(void)
            {
                return mux_handle.get_sample();
            }

            MediaFileInfo const & get_media_info(void) const
            {
                return mux_handle.get_media_info();
            }

            boost::uint64_t get_current_time()
            {
                return mux_handle.get_current_time();
            }

        private:
            handler mux_handle;
        };
    }
}
