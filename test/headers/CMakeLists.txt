add_library(threading-headers-tests STATIC EXCLUDE_FROM_ALL
        channel.cpp
        limiters.cpp
        map.cpp
        policy.cpp
        queue.cpp
        reactor.cpp
        ring.cpp
        set.cpp
        sync.cpp
    )
target_link_libraries(threading-headers-tests f5-threading boost)
add_dependencies(check threading-headers-tests)
