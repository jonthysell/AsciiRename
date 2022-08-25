cmake_minimum_required(VERSION 3.16.0)

add_subdirectory(libs/anyascii)
add_subdirectory(libs/libpu8)

project(ascii-rename VERSION 1.0.0)

add_executable(ascii-rename)

target_link_libraries(ascii-rename anyascii libpu8)

target_compile_definitions(ascii-rename PRIVATE VERSION_STR="${PROJECT_VERSION}")

target_include_directories(ascii-rename PRIVATE
    libs/anyascii
    libs/libpu8
    )

target_sources(ascii-rename PRIVATE
    src/main.cpp
    src/helpers.cpp
)

set_property(TARGET ascii-rename PROPERTY CXX_STANDARD 17)
