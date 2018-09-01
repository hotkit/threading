find_package(Threads REQUIRED)

function(runtest name)
    add_executable(threading-run-test-${name} EXCLUDE_FROM_ALL ${name}.cpp)
    target_link_libraries(threading-run-test-${name}
        ${CMAKE_THREAD_LIBS_INIT}
        boost_context
        boost_coroutine
        boost_system
        f5-threading
    )
    add_custom_command(TARGET threading-run-test-${name}
        POST_BUILD COMMAND threading-run-test-${name})
    if(NOT (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"))
        ## For some reason gcc won't actually run these tests. Until that
        ## is worked out we don't try to run them as part of the `checks`
        ## target.
        add_dependencies(check threading-run-test-${name})
        add_test(NAME threading-run-test-${name}-test COMMAND threading-run-test-${name})
    endif()
endfunction(runtest)

runtest(limiters-unlimited-blocking)
runtest(limiters-unlimited-nonblocking)
