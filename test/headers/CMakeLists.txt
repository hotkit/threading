add_library(threading-headers-tests STATIC EXCLUDE_FROM_ALL
        limiters.cpp
    )
target_link_libraries(threading-headers-tests f5-threading)
add_dependencies(check threading-headers-tests)
