cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0092 NEW)

project(Unixshox
  LANGUAGES C
)

include(cmake/compilerFlags.cmake REQUIRED)

add_executable(unishox
  unishox2.c 
  unishox2.h
  test_unishox2.c
)

include(cmake/summary.cmake REQUIRED)
