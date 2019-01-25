#ifndef WFP_COLLECTION
#define WFP_COLLECTION


#include <beanbag/jsondb_ptr.hpp>

#include <fost/array>


namespace wfp {


    /// Meta-data about all beanbags in the requested collection
    fostlib::json collection(
            f5::u8view user,
            const fostlib::string &name,
            const std::set<fostlib::string> &beanbags,
            fostlib::array_view<fostlib::jcursor> paths,
            fostlib::array_view<std::function<bool(const fostlib::json &)>> ifs);


    /// Return the requested database, or a new empty one if it doesn't exist
    boost::shared_ptr<fostlib::jsondb> database(
            f5::u8view user,
            f5::u8view collection,
            f5::u8view name,
            const fostlib::json &default_content = fostlib::json());


}


#endif
