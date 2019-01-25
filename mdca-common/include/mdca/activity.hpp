#include <fost/jsondb>
#include <fost/urlhandler>
#include <fost/datetime>

namespace mdca {


    class activity : public fostlib::urlhandler::view {
      public:
        activity(const fostlib::string &);

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const;

        /// Set true when any external activity is in progress;
        static fostlib::nullable<fostlib::timestamp> locked;
        static int sleeper;

      protected:
        static std::unique_ptr<fostlib::worker> process;

        /// Called during initial processing to clean up anything in the old value
        virtual void tidy_old_value(
                fostlib::json &result,
                const fostlib::json &config,
                const fostlib::json &body,
                fostlib::jsondb::local &trans,
                const fostlib::jcursor &path) const;

        /// Schedule the work for the activity so it starts. Return an error
        virtual fostlib::nullable<fostlib::string>
                post(fostlib::json &result,
                     const fostlib::json &config,
                     const fostlib::json &body,
                     boost::shared_ptr<fostlib::jsondb> dbp,
                     const fostlib::jcursor &path) const = 0;
    };


}
