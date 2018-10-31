find_package(Threads REQUIRED)

function(runtest name)
    add_executable(threading-run-test-${name} EXCLUDE_FROM_ALL ${name}.cpp)
    target_link_libraries(threading-run-test-${name}
        ${CMAKE_THREAD_LIBS_INIT}
        f5-threading
        boost
        boost_chrono
        boost_context
        boost_coroutine
        boost_system
        boost_thread
    )
    add_custom_command(TARGET threading-run-test-${name}
        POST_BUILD COMMAND threading-run-test-${name})
    add_dependencies(check threading-run-test-${name})
    add_test(NAME threading-run-test-${name}-test COMMAND threading-run-test-${name})
endfunction(runtest)

if(NOT (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"))
    ## For some reason gcc won't actually run these tests. Until that
    ## is worked out we don't try to run them as part of the `checks`
    ## target.
    ##
    ## What seems to happen is that these executables link only with
    ## boost_coroutine and boost_system, but the libraries that they use
    ## (for example boost.chrono) don't then get loaded as the dynamic
    ## doesn't seem to know where to find them. Setting `LD_LIBRARY_PATH`
    ## and then manually running the built binary works.
    runtest(limiters-unlimited-blocking)
    runtest(limiters-unlimited-nonblocking)
endif()
runtest(tsmap-unique_ptr)
