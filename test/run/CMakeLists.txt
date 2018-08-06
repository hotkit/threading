function(runtest name)
    add_executable(threading-run-test-${name} EXCLUDE_FROM_ALL ${name}.cpp)
    target_link_libraries(threading-run-test-${name} f5-threading boost_system)
    add_custom_command(TARGET threading-run-test-${name}
        POST_BUILD COMMAND threading-run-test-${name})
    add_dependencies(check threading-run-test-${name})
    add_test(NAME threading-run-test-${name}-test COMMAND threading-run-test-${name})
endfunction(runtest)

runtest(limiters-unlimited)
