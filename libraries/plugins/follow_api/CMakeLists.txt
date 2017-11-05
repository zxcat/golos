set(CURRENT_TARGET follow_api_plugin)

list(APPEND CURRENT_TARGET_HEADERS
     include/golos/plugins/follow_api/api.hpp
     include/golos/plugins/follow_api/api_plugin.hpp
     )

list(APPEND CURRENT_TARGET_SOURCES
     api.cpp
     api_plugin.cpp
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
        golos::chain
        golos::chain_plugin
        golos::protocol
        golos::follow_plugin
        golos::database_api_plugin
        appbase
        golos::json_rpc_plugin
        fc
)
target_include_directories(golos_${CURRENT_TARGET} PUBLIC "include")

install(TARGETS
        golos_${CURRENT_TARGET}

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
