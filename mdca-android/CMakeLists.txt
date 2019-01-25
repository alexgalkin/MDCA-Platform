add_library(mdca-android
        src/filesystem/directory_iterator.cpp
        src/filesystem/operations.cpp
    )
## Android filesystem files come from
## <https://android.googlesource.com/platform/external/libcxx/+/master/src/filesystem>
## It's highly likely that these need to be refreshed after every
## NDK release :-(

target_link_libraries(mdca-android
        beanbag-views
        fost-android-java
        fost-core
        fost-jsondb
        mdca-common
    )
