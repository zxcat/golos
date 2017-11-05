set(CURRENT_TARGET snapshot_plugin)

list(APPEND CURRENT_TARGET_HEADERS
     include/golos/plugins/snapshot/snapshot_plugin.hpp
     include/golos/plugins/snapshot/snapshot_state.hpp
     )

list(APPEND CURRENT_TARGET_SOURCES
     snapshot_plugin.cpp
     )

if(BUILD_SHARED_LIBRARIES)
    add_library(golos_${CURRENT_TARGET} SHARED
                ${CURRENT_TARGET_HEADERS}
                ${CURRENT_TARGET_SOURCES}
                )
else()
    add_library(golos_${CURRENT_TARGET} STATIC
                ${CURRENT_TARGET_HEADERS}
                ${CURRENT_TARGET_SOURCES}
                )
endif()

add_library(golos::${CURRENT_TARGET} ALIAS golos_${CURRENT_TARGET})
set_property(TARGET golos_${CURRENT_TARGET} PROPERTY EXPORT_NAME ${CURRENT_TARGET})

target_link_libraries(
        golos_${CURRENT_TARGET}
        appbase
        golos::chain
        golos::account_by_key_plugin
        fc
)

target_include_directories(golos_${CURRENT_TARGET}
                           PUBLIC "include")