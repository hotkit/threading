cmake_minimum_required(VERSION 3.6)
project(f5-threading)

add_library(f5-threading INTERFACE)
target_include_directories(f5-threading INTERFACE include)
target_link_libraries(f5-threading INTERFACE
#         boost
#         boost_chrono
#         boost_context
#         boost_coroutine
#         boost_system
#         boost_thread
    )
target_compile_definitions(f5-threading INTERFACE BOOST_COROUTINES_NO_DEPRECATION_WARNING)
target_compile_definitions(f5-threading INTERFACE BOOST_COROUTINE_NO_DEPRECATION_WARNING)
if(CMAKE_VERSION VERSION_LESS "3.8")
    message(WARNING "This verison of cmake is too old to request C++17 for f5-threading.")
    target_compile_options(f5-threading INTERFACE -std=c++17)
else()
    target_compile_features(f5-threading INTERFACE cxx_std_17)
endif()


enable_testing()
if(NOT TARGET check)
    add_custom_target(check)
    set_property(TARGET check PROPERTY EXCLUDE_FROM_ALL TRUE)
endif()
add_subdirectory(test)
