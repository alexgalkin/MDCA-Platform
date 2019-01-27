#include <fost/crypto>
#include <fost/file>
#include <fost/log>
#include <fost/urlhandler>
#include <mdca/mdca.hpp>


namespace {
    const class webcapture : fostlib::urlhandler::view {
      public:
        webcapture() : view("wfp.webcapture") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            if (req.query_string().as_string()) {
                auto document = fostlib::coerce<fostlib::string>(host)
                        + req.file_spec().underlying();
                fostlib::string expires, key_name, signature;
                const auto query = fostlib::coerce<f5::u8view>(
                        req.query_string().as_string());
                f5::u8view prefix = "?";

                for (auto query_pair : fostlib::splitter(query, '&')) {
                    const auto query_start = query_pair.substr(0, 3);

                    if (query_start == "_k=") {
                        key_name = req.query_string()["_k"].value();
                    } else if (query_start == "_s=") {
                        signature = req.query_string()["_s"].value();
                    } else if (query_start == "_e=") {
                        expires = req.query_string()["_e"].value();
                    } else {
                        document += prefix;
                        document += query_pair;
                        prefix = "&";
                    }
                }

                document += '\n';
                if (!expires.empty()) document += expires;

                fostlib::hmac digester(
                        fostlib::sha256, wfp::c_backend_secret.value());
                digester << document;
                auto bytes = digester.digest();
                auto check = fostlib::coerce<fostlib::base64_string>(bytes);

                const bool match =
                        fostlib::crypto_compare(
                                signature, fostlib::string(check).substr(0, 43))
                        || fostlib::crypto_compare(
                                   signature, fostlib::string(check));

                auto jwt = fostlib::jwt::mint(fostlib::jwt::alg::HS256);
                jwt.subject(key_name);

                if (match) {
                    auto webcapture_index_string = fostlib::utf::load_file(
                            fostlib::coerce<boost::filesystem::path>(
                                    config["filepath"]));
                    auto const token =
                            jwt.token(wfp::c_jwt_secret.value().data());
                    auto index_to_serve = fostlib::replace_all(
                            webcapture_index_string,
                            fostlib::coerce<f5::u8view>(config["jwt-replace"]),
                            f5::u8view(token.data(), token.size()));

                    boost::shared_ptr<fostlib::mime> response(
                            new fostlib::text_body(
                                    index_to_serve,
                                    fostlib::mime::mime_headers(),
                                    "text/html"));
                    return std::make_pair(response, 200);
                } else {
                    fostlib::log::warning(wfp::c_mdca)(
                            "",
                            "Signatures do not match")("document", document)(
                            "signature", "request", signature)(
                            "signature", "generated", f5::u8view(check));
                    return execute(config["error"], path, req, host);
                }
            }

            throw fostlib::exceptions::not_implemented("Not implemented");
        }

    } c_webcapture;
}
