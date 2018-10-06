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
    add_dependencies(check threading-run-test-${name})
    add_test(NAME threading-run-test-${name}-test COMMAND threading-run-test-${name})
endfunction(runtest)

runtest(limiters-unlimited-blocking)
runtest(limiters-unlimited-nonblocking)
runtest(tsmap-unique_ptr)
