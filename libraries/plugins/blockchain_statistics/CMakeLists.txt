set(CURRENT_TARGET blockchain_statistics)

list(APPEND CURRENT_TARGET_HEADERS
    include/steemit/blockchain_statistics/blockchain_statistics_api.hpp
    include/steemit/blockchain_statistics/blockchain_statistics_plugin.hpp
    include/steemit/blockchain_statistics/statistics_sender.hpp
    include/steemit/blockchain_statistics/concurrent_queue.hpp
)

list(APPEND CURRENT_TARGET_SOURCES
    blockchain_statistics_plugin.cpp
    blockchain_statistics_api.cpp
    statistics_sender.cpp
)
if(BUILD_SHARED_LIBRARIES)
    add_library(golos_blockchain_statistics SHARED
        ${CURRENT_TARGET_HEADERS}
        ${CURRENT_TARGET_SOURCES}
    )
else()
    add_library(golos_blockchain_statistics STATIC
        ${CURRENT_TARGET_HEADERS}
        ${CURRENT_TARGET_SOURCES}
    )
endif()

add_library(golos::${CURRENT_TARGET} ALIAS golos_${CURRENT_TARGET})

target_link_libraries(
    golos_${CURRENT_TARGET}
    golos_chain
    golos_protocol
    golos_app
)

target_include_directories(golos_${CURRENT_TARGET}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../"
)

install(TARGETS
    golos_${CURRENT_TARGET}

    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# TODO
# Maybe we should also do smthng like this. 
# install(FILES ${HEADERS} DESTINATION "include/golos/chain_plugin")
