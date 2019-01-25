#include <mdca/collection.hpp>
#include <mdca/collection-view.hpp>
#include <mdca/mdca.hpp>
#include <beanbag/beanbag>
#include <fost/insert>
#include <fost/log>


using namespace fostlib;


namespace {


    const module c_collection(wfp::c_mdca, "collection-view");


    const struct wfp::collection_view c_collection_view("wfp.collection");


}


std::pair<boost::shared_ptr<mime>, int> wfp::collection_view::operator()(
        const json &options,
        const string &pathname,
        http::server::request &req,
        const host &h) const {
    std::pair<string, nullable<string>> parts(partition(pathname, "/"));
    if (parts.first.empty()) {
        std::set<fostlib::string> bags;
        for (const auto &b : req.query_string().at("b")) {
            if (b) { bags.insert(b.value()); }
        }
        std::vector<fostlib::jcursor> paths;
        for (const auto &v : req.query_string().at("q")) {
            if (v) { paths.push_back(beanbag::path_to_jcursor(v.value())); }
        }
        std::vector<std::function<bool(const fostlib::json &)>> ifs;
        auto keys = req.query_string().at("if-key"),
             values = req.query_string().at("if-value");
        for (std::size_t index = 0u; index < keys.size(); ++index) {
            if (index >= values.size() || not keys[index]
                || not values[index]) {
                throw fostlib::exceptions::not_implemented(
                        __func__,
                        "Missing if-value for if-key query, or missing if-key "
                        "or if-value value");
            } else {
                ifs.push_back(
                        [key = beanbag::path_to_jcursor(keys[index].value()),
                         value = values[index].value()](
                                const fostlib::json &js) {
                            try {
                                if (js.has_key(key)) {
                                    auto v = fostlib::coerce<
                                            fostlib::nullable<fostlib::string>>(
                                            js[key]);
                                    if (not v)
                                        return false;
                                    else
                                        return v.value() == value;
                                } else {
                                    return false;
                                }
                            } catch (fostlib::exceptions::exception &e) {
                                fostlib::insert(e.data(), "if-key", key);
                                fostlib::insert(e.data(), "if-value", value);
                                throw;
                            }
                        });
            }
        }
        mime::mime_headers headers;
        return std::make_pair(
                json_response(
                        options,
                        wfp::collection(
                                req.headers()["__jwt"].value(),
                                coerce<string>(options["collection"]), bags,
                                paths, ifs),
                        headers, jcursor()),
                200);
    } else {
        json raw_options(options);
        insert(raw_options, "database", parts.first);
        return (raw_view::operator())(
                raw_options, parts.second.value_or(string()), req, h);
    }
}


boost::shared_ptr<jsondb> wfp::collection_view::database(
        const json &options,
        const string &pathname,
        http::server::request &req,
        const host &) const {
    return wfp::database(
            req.headers()["__jwt"].value(),
            coerce<f5::u8view>(options["collection"]),
            coerce<f5::u8view>(options["database"]));
}
