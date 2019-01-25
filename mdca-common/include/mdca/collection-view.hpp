#include <beanbag/beanbag>

using namespace fostlib;

namespace wfp {


    struct collection_view : public beanbag::raw_view {
        collection_view(const string &name) : raw_view(name) {}

        /// The standard view implementation operator
        std::pair<boost::shared_ptr<mime>, int> operator()(
                const json &options,
                const string &pathname,
                http::server::request &req,
                const host &) const override;

        /// Return the database to use
        boost::shared_ptr<jsondb> database(
                const json &options,
                const string &pathname,
                http::server::request &req,
                const host &) const override;
    };


}
