cmake_minimum_required(VERSION 3.20)
project(GGDollDAW VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(GGDOLL_BUILD_TESTS "Build GG Doll tests" ON)

add_library(ggdoll_core
    src/domain/project_types.h
    src/domain/slice.h
    src/security/import_validator.h
    src/security/import_validator.cpp
    src/app/import_audio_use_case.h
    src/app/import_audio_use_case.cpp
)

target_include_directories(ggdoll_core PUBLIC src)

target_compile_options(ggdoll_core PRIVATE -Wall -Wextra -Wpedantic)

if (GGDOLL_BUILD_TESTS)
  enable_testing()
  add_executable(ggdoll_core_tests
      tests/unit/import_validator_test.cpp
  )
  target_link_libraries(ggdoll_core_tests PRIVATE ggdoll_core)
  add_test(NAME ggdoll_core_tests COMMAND ggdoll_core_tests)
endif()
