#include <mdca/collection.hpp>
#include <fost/file>
#include <fost/log>
#include <fost/test>


FSL_TEST_SUITE(collection);


FSL_TEST_FUNCTION(empty) {
    f5::u8view user;
    std::set<fostlib::string> bags;
    std::vector<fostlib::jcursor> paths;
    std::vector<std::function<bool(const fostlib::json &)>> ifs;
    FSL_CHECK_EQ(
            wfp::collection(user, "empty", bags, paths, ifs),
            fostlib::json::object_t());
}


FSL_TEST_FUNCTION(collection_for_one_db) {
    std::set<fostlib::string> bags;
    std::vector<fostlib::jcursor> paths;
    std::vector<std::function<bool(const fostlib::json &)>> ifs;
    boost::shared_ptr<fostlib::jsondb> dbp{wfp::database("u1", "c1", "db1")};
    fostlib::json meta{wfp::collection("u1", "c1", bags, paths, ifs)};
    fostlib::log::debug()("meta", meta);
    FSL_CHECK(meta.has_key("db1"));
    FSL_CHECK(meta["db1"]["stat"].has_key("modified"));
#ifdef DEBUG
    FSL_CHECK_EQ(fostlib::coerce<int>(meta["db1"]["stat"]["size"]), 5);
#else
    FSL_CHECK_EQ(fostlib::coerce<int>(meta["db1"]["stat"]["size"]), 4);
#endif
}


FSL_TEST_FUNCTION(collection_filters_non_databases) {
    std::set<fostlib::string> bags;
    std::vector<fostlib::jcursor> paths;
    std::vector<std::function<bool(const fostlib::json &)>> ifs;
    boost::shared_ptr<fostlib::jsondb> dbp{wfp::database("u1", "c2", "db1")};
    fostlib::utf::save_file(
            fostlib::jsondb::get_db_path(
                    "db/"
                    "bb82030dbc2bcaba32a90bf2e207a84a856fc5f033b77c480836ab6f77"
                    "f40f19/c2/file.txt"),
            "");
    fostlib::json meta{wfp::collection("u1", "c2", bags, paths, ifs)};
    fostlib::log::debug()("meta", meta);
    FSL_CHECK(meta.has_key("db1"));
    FSL_CHECK(not meta.has_key("file"));
}


FSL_TEST_FUNCTION(add_new_db) {
    boost::shared_ptr<fostlib::jsondb> dbp{
            wfp::database("u1", "one", "db-name")};
    FSL_CHECK(dbp.get());
    FSL_CHECK(dbp->filename());
    FSL_CHECK(fostlib::coerce<fostlib::string>(dbp->filename().value())
                      .endswith("/one/db-name.json"));
}


FSL_TEST_FUNCTION(get_existing_db) {
    boost::shared_ptr<fostlib::jsondb> dbp{wfp::database("u1", "c1", "db2")};
    FSL_CHECK_EQ(dbp, wfp::database("u1", "c1", "db2"));
}


FSL_TEST_FUNCTION(get_object_from_db) {
    boost::shared_ptr<fostlib::jsondb> dbp{wfp::database("u1", "c3", "db3")};
    fostlib::jsondb::local db_transaction(*dbp);
    FSL_CHECK_EQ(db_transaction[fostlib::jcursor()], fostlib::json());

    db_transaction.insert("obj1", "value1");
    FSL_CHECK_EQ(db_transaction["obj1"], fostlib::json{"value1"});
    db_transaction.commit();

    fostlib::jsondb::local t2(*dbp);
    FSL_CHECK_EQ(t2["obj1"], fostlib::json("value1"));
}
