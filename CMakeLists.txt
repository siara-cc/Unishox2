cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0092 NEW)

project(Unixshox
  LANGUAGES C
  VERSION 2.0.0
)

include(GNUInstallDirs)
include(WriteBasicConfigVersionFile)
include(CMakePackageConfigHelpers)
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

# --------------------------------------------------------------------
# Install rules for C library
# --------------------------------------------------------------------

write_basic_package_version_file(unishoxConfigVersion.cmake COMPATIBILITY ExactVersion)

install(TARGETS unishox EXPORT unishoxConfig
  RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/unishox/
)

install(FILES ${PROJECT_BINARY_DIR}/unishox_export.h unishox2.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/unishox)

install(EXPORT unishoxConfig DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/unishox")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/unishoxConfigVersion.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/unishox")

# --------------------------------------------------------------------
# Test CLI application 
# --------------------------------------------------------------------

add_executable(unishox_cli
  test_unishox2.c
)
target_link_libraries(unishox_cli PRIVATE unishox)

include(cmake/summary.cmake REQUIRED)
