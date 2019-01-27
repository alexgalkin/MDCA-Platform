add_library(mdca-common
        src/activity.cpp
        src/beanbag-view.cpp
        src/collection-view.cpp
        src/collection.cpp
        src/mdca.cpp
        src/secure.cpp
        src/webcapture.cpp
    )
target_include_directories(mdca-common PUBLIC include)
target_link_libraries(mdca-common beanbag-views)
if("${ANDROID_ABI}" STREQUAL "" AND NOT APPLE)
    # This is for libstd++ (i.e. GNU standard c++ library)
    target_link_libraries(mdca-common stdc++fs)
elseif(NOT "${ANDROID_ABI}" STREQUAL "")
    ## Because we have to back fill the filesystem implementation here we have the
    ## library dependency in an odd direction.
    target_link_libraries(mdca-common mdca-android)
endif()
set_target_properties(mdca-common PROPERTIES DEBUG_POSTFIX "-d")
install(TARGETS mdca-common LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

if(TARGET check)
    add_library(mdca-common-smoke STATIC EXCLUDE_FROM_ALL
            src/collection-tests.cpp
            src/collection-view-tests.cpp
            src/config-tests.cpp
            src/secure.tests.cpp
        )
    target_link_libraries(mdca-common-smoke mdca-common)
    smoke_test(mdca-common-smoke)
endif()
