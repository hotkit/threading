cmake_minimum_required(VERSION 3.6)
project(f5-threading)

add_library(f5-threading INTERFACE)
target_include_directories(f5-threading INTERFACE include)
if(CMAKE_VERSION VERSION_LESS "3.8")
    message(WARNING "This verison of cmake is too old to request C++17 for f5-threading.")
    target_compile_options(f5-threading INTERFACE -std=c++17)
else()
    target_compile_features(f5-threading INTERFACE cxx_std_17)
endif()

enable_testing()
