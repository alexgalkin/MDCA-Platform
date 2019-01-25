#include <mdca/mdca.hpp>


namespace {
    const fostlib::module c_wfp("wfp");
}


const fostlib::module wfp::c_mdca(c_wfp, "mdca");

const fostlib::setting<fostlib::string> wfp::c_jwt_secret(
        "wfp/wfp.cpp", "JWT", "Secret", "s8Yxb1ZxSUB+ImGcONc8OqS6rpYe54", true);

const fostlib::setting<fostlib::string> wfp::c_backend_secret(
        "wfp/wfp.cpp",
        "Backend",
        "Secret",
        "SECRET-TO-BE-REPLACED-FROM-PYTHON",
        true);
