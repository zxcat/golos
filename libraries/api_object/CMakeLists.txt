set(CURRENT_TARGET api_object)

list(APPEND ${CURRENT_TARGET}_HEADERS
        include/steemit/api_object/api_access.hpp
        include/steemit/api_object/api_context.hpp
        include/steemit/api_object/applied_operation.hpp
        include/steemit/api_object/database_api.hpp
        include/steemit/api_object/discussion_query.hpp
        include/steemit/api_object/impacted.hpp
        include/steemit/api_object/helper.hpp
        include/steemit/api_object/state.hpp
        include/steemit/api_object/steem_api_objects.hpp
        include/steemit/api_object/comment_api_obj.hpp
)

list(APPEND ${CURRENT_TARGET}_SOURCES
        discussion_query.cpp
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

add_library(golos::${CURRENT_TARGET} ALIAS golos_${CURRENT_TARGET} )
set_property(TARGET golos_${CURRENT_TARGET} PROPERTY EXPORT_NAME ${CURRENT_TARGET})

target_link_libraries(golos_${CURRENT_TARGET} fc graphene_utilities )

target_include_directories(golos_${CURRENT_TARGET}
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")



install(TARGETS
        golos_${CURRENT_TARGET}

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

install(FILES ${${CURRENT_TARGET}_HEADERS} DESTINATION "include/steemit/${CURRENT_TARGET}")