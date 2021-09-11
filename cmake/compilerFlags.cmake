include(CheckCCompilerFlag)



set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

if (${CMAKE_C_COMPILER_ID} STREQUAL GNU)
    set(COMPILER_IS_GCC ON)
elseif (${CMAKE_C_COMPILER_ID} MATCHES "Clang")
    set(COMPILER_IS_CLANG ON)
endif()

if (COMPILER_IS_GCC OR COMPILER_IS_CLANG)
    add_compile_options(-Werror)
    add_compile_options(-Wall -Wcast-align -Wpointer-arith -Wformat-security -Wmissing-format-attribute -W)
    add_compile_options(-Wno-error=format-nonliteral)

    check_c_compiler_flag(-Wdeprecated-copy DEPRECATED_COPY)
    if ( DEPRECATED_COPY)
        add_compile_options(-Wdeprecated-copy)
    endif ()

endif()

if (MSVC)
    add_compile_options(/MP)
    add_compile_options(/W3 /WX)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_definitions(-wd5105)
endif()
