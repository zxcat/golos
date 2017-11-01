set(CURRENT_TARGET chain_plugin)
list(APPEND CURRENT_TARGET_HEADERS
        include/steemit/plugins/chain/chain_plugin.hpp
)

list(APPEND CURRENT_TARGET_SOURCES
        chain_plugin.cpp
)

if (BUILD_SHARED_LIBRARIES)
    add_library(golos_${CURRENT_TARGET} SHARED
            ${CURRENT_TARGET_HEADERS}
            ${CURRENT_TARGET_SOURCES}
            )
else ()
    add_library(golos_${CURRENT_TARGET} STATIC
            ${CURRENT_TARGET_HEADERS}
            ${CURRENT_TARGET_SOURCES}
            )
endif ()

add_library(golos::${CURRENT_TARGET} ALIAS golos_${CURRENT_TARGET})

set_property(TARGET golos_${CURRENT_TARGET} PROPERTY EXPORT_NAME ${CURRENT_TARGET})

target_link_libraries(
        golos_${CURRENT_TARGET}
        golos::chain
        golos::protocol
        fc
        appbase
)
target_include_directories( golos_${CURRENT_TARGET}
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/../../")

install(TARGETS
        golos_${CURRENT_TARGET}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
install( FILES ${HEADERS} DESTINATION "include/steemit/chain_plugin" )
