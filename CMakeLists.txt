cmake_minimum_required(VERSION 3.14)
project(lz-compressor VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Library
add_library(lzlib
    src/lz77.c
    src/huffman.c
    src/deflate.c
    src/stream.c
)
target_include_directories(lzlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# CLI tool
add_executable(lz-compressor src/main.c)
target_link_libraries(lz-compressor PRIVATE lzlib)

# Tests
enable_testing()

add_executable(test_lz77 test/test_lz77.c)
target_link_libraries(test_lz77 PRIVATE lzlib)
add_test(NAME test_lz77 COMMAND test_lz77)

add_executable(test_huffman test/test_huffman.c)
target_link_libraries(test_huffman PRIVATE lzlib)
add_test(NAME test_huffman COMMAND test_huffman)

add_executable(test_deflate test/test_deflate.c)
target_link_libraries(test_deflate PRIVATE lzlib)
add_test(NAME test_deflate COMMAND test_deflate)

add_executable(test_stream test/test_stream.c)
target_link_libraries(test_stream PRIVATE lzlib)
add_test(NAME test_stream COMMAND test_stream)
