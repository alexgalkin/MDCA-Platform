#include <fost/file>
#include <fost/insert>
#include <fost/jsondb>


namespace {


    const fostlib::setting<fostlib::string> c_data_root(
            "config-tests.cpp",
            fostlib::c_jsondb_root,
            fostlib::coerce<fostlib::string>(fostlib::unique_filename()));


}
