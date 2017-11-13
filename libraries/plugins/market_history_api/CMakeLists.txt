set(CURRENT_TARGET market_history_api)

list(APPEND CURRENT_TARGET_HEADERS
     include/golos/plugins/${CURRENT_TARGET}/api.hpp
     include/golos/plugins/${CURRENT_TARGET}/api_plugin.hpp
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
        golos_market_history_plugin
        golos_chain_plugin
        golos_json_rpc_plugin
        golos_database_api_plugin
)

target_include_directories(golos_${CURRENT_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/../../")

install(TARGETS
        golos_${CURRENT_TARGET}

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
