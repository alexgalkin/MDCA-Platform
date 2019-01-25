#include <mdca/collection.hpp>
#include <beanbag/beanbag>


namespace {


    const class bbv : public beanbag::raw_view {
      public:
        bbv() : raw_view("wfp.beanbag") {}

        beanbag::jsondb_ptr database(
                const fostlib::json &options,
                const fostlib::string &pathname,
                fostlib::http::server::request &req,
                const fostlib::host &) const override {
            return wfp::database(
                    req.headers()["__jwt"].value(), "mdca",
                    fostlib::coerce<f5::u8view>(options["database"]["name"]),
                    options["database"]["initial"]);
        }
    } c_wfp_bbv;


}
