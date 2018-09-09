cmake_minimum_required(VERSION 3.8)
project(sqfvm)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MACOSX_RPATH 1)

set(ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)
set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)

include_directories("${PROJECT_SOURCE_DIR}/include/tclap-1.2.2/include")

file(GLOB vm_src "${PROJECT_SOURCE_DIR}/src/*.h" "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.c")

find_package(Threads)
add_executable(sqfvm ${vm_src})
target_link_libraries(sqfvm ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_library(libsqfvm SHARED ${vm_src})

include(ExternalProject)

# Which version of JSON to build against, branch/commit/tag
set(JSON_VERSION "v3.2.0")

# Download JSON library
ExternalProject_Add(
  json
  PREFIX ${CMAKE_CURRENT_LIST_DIR}/vendor/json
  URL https://github.com/nlohmann/json/archive/${JSON_VERSION}.tar.gz
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
)

include_directories("${PROJECT_SOURCE_DIR}/vendor/json/src/json/include")
