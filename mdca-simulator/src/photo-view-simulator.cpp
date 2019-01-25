#include <mdca/activity.hpp>
#include <mdca/collection.hpp>
#include <beanbag/beanbag>
#include <fost/crypto>
#include <fost/datetime>
#include <fost/insert>
#include <fost/log>
#include <fost/push_back>


using namespace fostlib;


namespace {


    const struct photo : public mdca::activity {
        photo() : activity("wfp.photo.simulator") {}

        nullable<string>
                post(fostlib::json &result,
                     const json &config,
                     const json &body,
                     boost::shared_ptr<fostlib::jsondb> dbp,
                     const fostlib::jcursor &path) const {
            if (not process) { process.reset(new worker); }
            (*process)([this, config, dbp, path]() {
                try {
                    const auto folder =
                            fostlib::coerce<boost::filesystem::path>(
                                    config["folder"]);
                    auto source = folder
                            / fostlib::coerce<boost::filesystem::path>(
                                          config["source"]);
                    auto base = boost::filesystem::unique_path();
                    auto value =
                            (folder / base).replace_extension(".thumb.jpeg");
                    auto original = (folder / base).replace_extension(".jpeg");
                    sleep(5);

                    boost::filesystem::copy(source, value);
                    boost::filesystem::copy(source, original);
                    fostlib::digester d(fostlib::md5);
                    d << original;
                    auto md5 = fostlib::coerce<fostlib::string>(
                            fostlib::coerce<fostlib::base64_string>(d.digest()));

                    jsondb::local trans(*dbp);
                    trans.set(path / "value", value)
                            .set(path / "original", original)
                            .set(path / "md5", md5)
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
    } c_photo_simulator;


}
