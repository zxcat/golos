add_executable(golosd main.cpp)

if(UNIX AND NOT APPLE)
    set(rt_library rt)
elseif(APPLE)
    list(APPEND PLATFORM_SPECIFIC_LIBS readline)
endif()

find_package(Gperftools QUIET)
if(GPERFTOOLS_FOUND)
    message(STATUS "Found gperftools; compiling golosd with TCMalloc")
    list(APPEND PLATFORM_SPECIFIC_LIBS tcmalloc)
endif()

target_link_libraries(golosd PRIVATE
        appbase
        golos::webserver_plugin
        golos::p2p
        graphene_utilities
        golos::chain_plugin
        golos::network_broadcast_api_plugin
        golos_protocol
        fc
        ${CMAKE_DL_LIBS}
        ${PLATFORM_SPECIFIC_LIBS}
)

install(TARGETS
        golosd

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
