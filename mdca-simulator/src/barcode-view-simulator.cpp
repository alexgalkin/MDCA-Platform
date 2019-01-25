#include <mdca/activity.hpp>
#include <mdca/collection.hpp>
#include <beanbag/beanbag>
#include <fost/datetime>
#include <fost/insert>
#include <fost/log>
#include <fost/push_back>


using namespace fostlib;


namespace {


    const struct barcode : public mdca::activity {
        barcode() : activity("wfp.barcode.simulator") {}

        nullable<string>
                post(fostlib::json &result,
                     const json &config,
                     const json &body,
                     boost::shared_ptr<fostlib::jsondb> dbp,
                     const fostlib::jcursor &path) const {
            if (not process) { process.reset(new worker); }
            (*process)([this, config, dbp, path]() {
                try {
                    sleep(5);
                    jsondb::local trans(*dbp);
                    trans.set(path / "value", config["barcode"])
                            .set(path / "completed", timestamp::now())
                            .commit();
                } catch (std::exception &e) {
                    log::error()("photo-exception", e.what());
                    absorb_exception();
                }
                locked = fostlib::null;
            });
            return null;
        }
    } c_barcode_simulator;


}
