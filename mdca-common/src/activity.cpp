#include <mdca/activity.hpp>
#include <mdca/collection.hpp>
#include <mdca/mdca.hpp>
#include <fost/datetime>
#include <fost/insert>
#include <fost/log>
#include <fost/push_back>


using namespace fostlib;


namespace {
    const module c_activity(wfp::c_mdca, "activity");
}


std::unique_ptr<worker> mdca::activity::process;
fostlib::nullable<fostlib::timestamp> mdca::activity::locked = fostlib::null;
int mdca::activity::sleeper = 60; // Time to sleep for lock

mdca::activity::activity(const string &name) : view(name) {}


std::pair<boost::shared_ptr<mime>, int> mdca::activity::operator()(
        const json &config,
        const string &path,
        http::server::request &req,
        const host &host) const {
    log::debug(c_activity)("", "Activity operator ()")(
            "mdca::activity::locked", locked)("config", config)("path", path);
    if (locked) {
        json errors{json::array_t()};
        push_back(errors, "The API is already processing");
        boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                json::unparse(errors, false), fostlib::mime::mime_headers(),
                L"application/json"));
        return std::make_pair(response, 423);
    } else if (req.method() == "POST") {
        json errors{json::array_t()};
        json body(json::parse(
                coerce<string>(coerce<utf8_string>(req.data()->data()))));
        if (!config.has_key("collection")) {
            push_back(
                    errors, "Configuration doesn't specify a collection name");
        }
        if (!body.has_key("beanbag")) {
            push_back(
                    errors,
                    "Must include a beanbag key which contains the beanbag "
                    "name to use");
        }
        if (!body.has_key("path")) {
            push_back(
                    errors,
                    "Must include a path key which contains the jcursor for "
                    "putting the result into");
        }
        if (errors.size() == 0u) {
            locked = fostlib::timestamp::now();
            boost::shared_ptr<fostlib::jsondb> dbp(wfp::database(
                    config.has_key("user") ? coerce<string>(config["user"])
                                           : string{},
                    coerce<string>(config["collection"]),
                    coerce<string>(body["beanbag"])));
            jsondb::local trans(*dbp);
            jcursor path(coerce<jcursor>(body["path"]));
            json result;
            tidy_old_value(result, config, body, trans, path);
            trans.set(path / "started", timestamp::now()).commit();
            insert(result, "config", config);
            if (not process) { process.reset(new worker); }
            (*process)([wakeup = locked.value() + fostlib::seconds(60)]() {
                sleep(sleeper);
                if (not locked || locked.value() > fostlib::timestamp::now()) {
                    // Do nothing, either unlocked already, or not yet time
                    log::debug(c_activity)(
                            "", "Activity operator () Still Locked")(
                            "mdca::activity::locked",
                            locked)("wfp::activity::sleeper", sleeper)(
                            "Now", fostlib::timestamp::now());
                } else {
                    // Force unlock because we timed out
                    locked = fostlib::null;
                    log::debug(c_activity)(
                            "", "Activity operator () Now Unlocked")(
                            "mdca::activity::locked",
                            locked)("Now", fostlib::timestamp::now());
                }
            });
            auto error = post(result, config, body, dbp, path);
            if (error) { insert(result, "error", error.value()); }
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    json::unparse(result, false), fostlib::mime::mime_headers(),
                    L"application/json"));
            return std::make_pair(response, error ? 449 : 200);
        } else {
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    json::unparse(errors, false), fostlib::mime::mime_headers(),
                    L"application/json"));
            return std::make_pair(response, 449);
        }
    } else {
        json allow;
        push_back(allow, "allow", "POST");
        return urlhandler::response_405(allow, path, req, host);
    }
}


void mdca::activity::tidy_old_value(
        json &result,
        const json &config,
        const json &body,
        jsondb::local &trans,
        const jcursor &path) const {
    const jcursor path_value(path, "value");
    if (trans.has_key(path_value)) {
        insert(result, "old", "value", trans[path_value]);
        trans.remove(path_value);
    }
    const jcursor original_value(path, "original");
    if (trans.has_key(original_value)) {
        insert(result, "old", "original", trans[original_value]);
        trans.remove(original_value);
    }
    const jcursor md5_value(path, "md5");
    if (trans.has_key(md5_value)) {
        insert(result, "old", "md5", trans[md5_value]);
        trans.remove(md5_value);
    }
    const jcursor path_completed(path, "completed");
    if (trans.has_key(path_completed)) { trans.remove(path_completed); }
    const jcursor path_error(path, "error");
    if (trans.has_key(path_error)) { trans.remove(path_error); }
}
