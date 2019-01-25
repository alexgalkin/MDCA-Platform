add_library(mdca-simulator
        src/barcode-view-simulator.cpp
        src/photo-view-simulator.cpp
    )
target_link_libraries(mdca-simulator mdca-common)
set_target_properties(mdca-simulator PROPERTIES DEBUG_POSTFIX "-d")
install(TARGETS mdca-simulator LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
