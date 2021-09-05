cmake_minimum_required(VERSION 3.5)

project(Unixshox
  LANGUAGES C
)

include(cmake/compilerFlags.cmake)

add_executable(unishox
  unishox2.c 
  unishox2.h
  test_unishox2.c
)
