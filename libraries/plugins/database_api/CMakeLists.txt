set(CURRENT_TARGET database_api_plugin)

list(APPEND ${CURRENT_TARGET}_HEADERS
        include/steemit/plugins/database_api/applied_operation.hpp
        include/steemit/plugins/database_api/discussion_query.hpp
        include/steemit/plugins/database_api/state.hpp
        include/steemit/plugins/database_api/database_api.hpp
        include/steemit/plugins/database_api/database_api_plugins.hpp

        include/steemit/plugins/database_api/comment_api_obj.hpp
        include/steemit/plugins/database_api/category_api_obj.hpp
        include/steemit/plugins/database_api/account_api_obj.hpp
        include/steemit/plugins/database_api/account_recovery_request_api_obj.hpp
        include/steemit/plugins/database_api/feed_history_api_obj.hpp
        include/steemit/plugins/database_api/forward.hpp
        include/steemit/plugins/database_api/owner_authority_history_api_obj.hpp
        include/steemit/plugins/database_api/savings_withdraw_api_obj.hpp
        include/steemit/plugins/database_api/tag_api_obj.hpp
        include/steemit/plugins/database_api/witness_api_obj.hpp


)

list(APPEND ${CURRENT_TARGET}_SOURCES
        database_api_plugins.cpp
        discussion_query.cpp
        database_api.cpp
        database_api_plugin.cpp
        applied_operation.cpp
)

if (BUILD_SHARED_LIBRARIES)
    add_library(golos_${CURRENT_TARGET} SHARED
            ${${CURRENT_TARGET}_HEADERS}
            ${${CURRENT_TARGET}_SOURCES}
)
else ()
    add_library(golos_${CURRENT_TARGET} STATIC
            ${${CURRENT_TARGET}_HEADERS}
            ${${CURRENT_TARGET}_SOURCES}
)
endif ()

add_library(golos::${CURRENT_TARGET} ALIAS golos_${CURRENT_TARGET})
set_property(TARGET golos_${CURRENT_TARGET} PROPERTY EXPORT_NAME ${CURRENT_TARGET})

target_link_libraries(
        golos_${CURRENT_TARGET}
        golos::chain
        golos::chain_plugin
        golos::protocol
        golos::tags_plugin
        golos::market_history_plugin
        golos::snapshot_plugin
        golos::languages_plugin
        golos::follow_api_plugin
        golos::json_rpc_plugin
        graphene_utilities
        appbase
        fc
)
target_include_directories(golos_${CURRENT_TARGET} PUBLIC "include")

install(TARGETS
        golos_${CURRENT_TARGET}

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )