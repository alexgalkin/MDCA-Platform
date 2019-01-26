#include <beanbag/beanbag>
#include <fost/crypto>
#include <fost/datetime>
#include <fost/file>
#include <fost/insert>
#include <fost/log>
#include <fost/push_back>
#include <fost/urlhandler>

#if __has_include(<filesystem>) || __has_include(<experimental/filesystem>)
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
using error_code = std::error_code;
namespace {
    auto to_time_t(const fs::path &p) {
        auto ftime = fs::last_write_time(p);
        return decltype(ftime)::clock::to_time_t(ftime);
    }
}
#else
namespace fs = boost::filesystem;
using error_code = boost::system::error_code;
namespace {
    auto to_time_t(const boost::filesystem::path &p) {
        return fs::last_write_time(p);
    }
}
#endif

#include <mutex>

#include <mdca/collection.hpp>
#include <mdca/mdca.hpp>


namespace {
    // Mutex for controlling access to g_databases
    std::mutex g_mutex;
    // Databases grouped by collection name
    std::map<
            fostlib::string,
            std::map<
                    fostlib::string,
                    std::map<fostlib::string, beanbag::jsondb_ptr>>>
            g_databases;

    auto user_path(f5::u8view user, f5::u8view collection) {
        if (user.empty()) {
            return boost::filesystem::path("db")
                    / boost::filesystem::path(std::string(collection));
        } else {
            return boost::filesystem::path("db")
                    / fostlib::coerce<boost::filesystem::path>(
                              fostlib::sha256(user))
                    / boost::filesystem::path(std::string(collection));
        }
    }

    // Calculate the relative database file name to use
    auto filename(f5::u8view user, f5::u8view collection, f5::u8view name) {
        return (user_path(user, collection)
                / boost::filesystem::path(std::string(name)))
                .replace_extension(".json");
    }
}


fostlib::json wfp::collection(
        f5::u8view user,
        const fostlib::string &collection,
        const std::set<fostlib::string> &bags,
        fostlib::array_view<fostlib::jcursor> paths,
        fostlib::array_view<std::function<bool(const fostlib::json &)>> ifs) {
    fostlib::json beanbags{fostlib::json::object_t()};
    error_code error;
    // The directory might not exist. Error handling deals with that
    fs::directory_iterator dirend,
            entry(fostlib::jsondb::get_db_path(user_path(user, collection))
                          .native(),
                  error);
    if (not error) {
        for (; entry != dirend; ++entry) {
            auto logger = fostlib::log::debug(c_mdca);
            logger("", "Directory iteration file")(
                    "filename", entry->path().c_str());
            if (entry->path().extension() == ".json") {
                const fostlib::string stem{fostlib::coerce<fostlib::string>(
                        entry->path().stem().c_str())};
                bool include = (bags.size() == 0 || bags.count(stem));
                logger("include", include);
                if (paths.size() || ifs.size()) {
                    try {
                        auto database_ptr =
                                wfp::database(user, collection, stem);
                        fostlib::jsondb::local database(*database_ptr);
                        for (const auto &condition : ifs) {
                            include = include & condition(database.data());
                        }
                        if (include) {
                            for (const auto &path : paths) {
                                if (database.has_key(path)) {
                                    fostlib::insert(
                                            beanbags, stem, path,
                                            database[path]);
                                }
                            }
                        }
                    } catch (fostlib::exceptions::exception &e) {
                        fostlib::insert(e.data(), "beanbag", stem);
                        throw;
                    }
                }
                if (include) {
                    fostlib::insert(
                            beanbags, stem, "stat", "size",
                            fs::file_size(entry->path()));
                    auto const tftime = to_time_t(entry->path());
                    fostlib::insert(
                            beanbags, stem, "stat", "modified",
                            fostlib::timestamp(1970, 1, 1, 0, 0, tftime));
                }
            }
        }
    }
    return beanbags;
}


beanbag::jsondb_ptr wfp::database(
        f5::u8view user,
        f5::u8view collection,
        f5::u8view name,
        const fostlib::json &default_content) {
    std::lock_guard<std::mutex> lock(g_mutex);
    beanbag::jsondb_ptr dbp{g_databases[user][collection][name]};
    if (not dbp) {
        dbp = g_databases[user][collection][name] =
                boost::make_shared<fostlib::jsondb>(
                        filename(user, collection, name), default_content);
    }
    return dbp;
}
