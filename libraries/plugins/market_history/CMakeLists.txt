set(CURRENT_TARGET market_history_plugin)

list(APPEND ${CURRENT_TARGET}_HEADERS
        include/steemit/plugins/market_history/bucket_object.hpp
        include/steemit/plugins/market_history/key_interface.hpp
        include/steemit/plugins/market_history/market_history_object_types.hpp
        include/steemit/plugins/market_history/market_history_plugin.hpp
        include/steemit/plugins/market_history/order_history_object.hpp
)

list(APPEND ${CURRENT_TARGET}_SOURCES
        bucket_object.cpp
        key_interface.cpp
        order_history_object.cpp
        market_history_plugin.cpp
)

if(BUILD_SHARED_LIBRARIES)
    add_library(golos_${CURRENT_TARGET} SHARED
            ${${CURRENT_TARGET}_HEADERS}
            ${${CURRENT_TARGET}_SOURCES}
    )
else()
    add_library(golos_${CURRENT_TARGET} STATIC
            ${${CURRENT_TARGET}_HEADERS}
            ${${CURRENT_TARGET}_SOURCES}
    )
endif()

add_library(golos::${CURRENT_TARGET} ALIAS golos_${CURRENT_TARGET})
set_property(TARGET golos_${CURRENT_TARGET} PROPERTY EXPORT_NAME ${CURRENT_TARGET})

target_link_libraries(
        golos_${CURRENT_TARGET}
        golos::chain
        golos::chain_plugin
        golos::protocol
        appbase
)
target_include_directories(golos_${CURRENT_TARGET}
        PUBLIC "include")

install(TARGETS
        golos_${CURRENT_TARGET}

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )