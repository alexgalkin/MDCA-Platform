#include <fost/crypto>
#include <fost/insert>
#include <fost/log>
#include <fost/urlhandler>
#include <mdca/mdca.hpp>


namespace {


    const class secure : public fostlib::urlhandler::view {
      public:
        secure() : view("wfp.secure") {}


        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            // Now check which sub-view to enter
            if (req.headers().exists("Authorization")) {
                auto parts = fostlib::partition(
                        req.headers()["Authorization"].value(), " ");
                if (parts.first == "Bearer" && parts.second) {
                    auto jwt = fostlib::jwt::token::load(
                            wfp::c_jwt_secret.value(), parts.second.value());
                    if (jwt) {
                        fostlib::log::debug(wfp::c_mdca)(
                                "", "JWT authenticated")(
                                "header", jwt.value().header)(
                                "payload", jwt.value().payload);
                        if (jwt.value().payload.has_key("sub")) {
                            req.headers().set(
                                    "__jwt", jwt.value().payload, "sub");
                            req.headers().set(
                                    "__user",
                                    fostlib::coerce<fostlib::string>(
                                            jwt.value().payload["sub"]));
                        }
                        return execute(config["secure"], path, req, host);
                    }
                } else {
                    fostlib::log::warning(wfp::c_mdca)(
                            "", "Invalid Authorization scheme")(
                            "scheme", parts.first)("data", parts.second);
                }
            }
            return execute(config["unsecure"], path, req, host);
        }
    } c_secure;


}
