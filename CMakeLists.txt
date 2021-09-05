cmake_minimum_required(VERSION 3.5)

project(Unixshox
  LANGUAGES C
)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

add_executable(unishox
  unishox2.c 
  unishox2.h
  test_unishox2.c
)
