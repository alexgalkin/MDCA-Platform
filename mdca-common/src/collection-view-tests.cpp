#include <mdca/collection.hpp>
#include <mdca/collection-view.hpp>
#include <fost/crypto>
#include <fost/http.server.hpp>
#include <fost/internet>
#include <fost/test>
#include <mdca/mdca.hpp>


using namespace fostlib;


FSL_TEST_SUITE(collection_view);


namespace {
    struct env {
        const struct wfp::collection_view view;
        std::unique_ptr<binary_body> body;
        string collection;
        json configuration;

        env()
        : view("wfp.collection"),
          body(std::make_unique<binary_body>(std::vector<unsigned char>())),
          collection("collection-view-tests") {
            insert(configuration, "collection", collection);
        }
    };
}


FSL_TEST_FUNCTION(empty) {
    env setup;
    http::server::request req("GET", "/", std::move(setup.body));
    auto jwt = fostlib::jwt::mint(fostlib::jwt::alg::HS256);
    jwt.subject("test-user");
    req.headers().set(
            "Authorization",
            fostlib::string(
                    "Bearer " + jwt.token(wfp::c_jwt_secret.value().data())));
    std::pair<boost::shared_ptr<mime>, int> response(
            setup.view(setup.configuration, "", req, host()));

    FSL_CHECK_EQ(response.second, 200);
}


FSL_TEST_FUNCTION(get_database) {
    env setup;
    wfp::database("test-user", setup.collection, "get-database");
    http::server::request req("GET", "/get-database/", std::move(setup.body));
    auto jwt = fostlib::jwt::mint(fostlib::jwt::alg::HS256);
    jwt.subject("test-user");
    req.headers().set(
            "Authorization",
            fostlib::string(
                    "Bearer " + jwt.token(wfp::c_jwt_secret.value().data())));
    auto response(
            setup.view(setup.configuration, "get-database/", req, host()));

    FSL_CHECK_EQ(response.second, 200);
}
