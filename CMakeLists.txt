cmake_minimum_required(VERSION 3.16.0)

add_subdirectory(libs/anyascii)

project(ascii-rename VERSION 0.0.1)

add_executable(ascii-rename)

target_link_libraries(ascii-rename anyascii)

target_compile_definitions(ascii-rename PRIVATE VERSION_STR="${PROJECT_VERSION}")

target_include_directories(ascii-rename PRIVATE libs/anyascii)

target_sources(ascii-rename PRIVATE
    src/main.cpp
)

set_property(TARGET ascii-rename PROPERTY CXX_STANDARD 17)
