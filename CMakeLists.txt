cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0092 NEW)

project(Unixshox
  LANGUAGES C
)

include(GenerateExportHeader)
include(cmake/compilerFlags.cmake REQUIRED)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

include_directories(${CMAKE_BINARY_DIR})

add_library(unishox
  unishox2.h
  unishox2.c
)

generate_export_header(unishox)

add_executable(unishox_cli
  test_unishox2.c
)
target_link_libraries(unishox_cli PRIVATE unishox)

include(cmake/summary.cmake REQUIRED)
