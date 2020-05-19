cmake_minimum_required(VERSION 3.10)
project(f5-threading-circleci)

add_custom_target(check)

add_subdirectory(boost)
add_subdirectory(.. threading)

