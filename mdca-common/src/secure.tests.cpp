#include <fost/crypto>
#include <fost/test>
#include <fost/urlhandler>
#include <mdca/mdca.hpp>


namespace {
    auto secure_config() {
        fostlib::json config;
        fostlib::insert(config, "view", "wfp.secure");
        fostlib::insert(config, "configuration", "secure", "fost.response.200");
        fostlib::insert(
                config, "configuration", "unsecure", "fost.response.403");
        return config;
    }
}


FSL_TEST_SUITE(secure);


FSL_TEST_FUNCTION(unsecure_no_jwt) {
    fostlib::http::server::request req("GET", "/");
    auto result = fostlib::urlhandler::view::execute(
            secure_config(), "", req, fostlib::host());
    FSL_CHECK_EQ(result.second, 403);
}


FSL_TEST_FUNCTION(unsecure_bad_authorization) {
    fostlib::http::server::request req("GET", "/");
    req.headers().set("Authorization", "Bearer ABC");
    auto result = fostlib::urlhandler::view::execute(
            secure_config(), "", req, fostlib::host());
    FSL_CHECK_EQ(result.second, 403);
}


FSL_TEST_FUNCTION(secure_no_subject) {
    fostlib::http::server::request req("GET", "/");
    auto jwt = fostlib::jwt::mint(fostlib::jwt::alg::HS256);
    req.headers().set(
            "Authorization",
            fostlib::string(
                    "Bearer " + jwt.token(wfp::c_jwt_secret.value().data())));
    auto result = fostlib::urlhandler::view::execute(
            secure_config(), "", req, fostlib::host());
    FSL_CHECK_EQ(result.second, 200);
}


FSL_TEST_FUNCTION(secure_with_subject) {
    fostlib::http::server::request req("GET", "/");
    auto jwt = fostlib::jwt::mint(fostlib::jwt::alg::HS256);
    jwt.subject("test-user");
    req.headers().set(
            "Authorization",
            fostlib::string(
                    "Bearer " + jwt.token(wfp::c_jwt_secret.value().data())));
    auto result = fostlib::urlhandler::view::execute(
            secure_config(), "", req, fostlib::host());
    FSL_CHECK_EQ(result.second, 200);
}
