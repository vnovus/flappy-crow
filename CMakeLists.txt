cmake_minimum_required(VERSION 3.16)
project(flappy-crow VERSION 0.1.1 LANGUAGES C)

# Set project properties.
set(CMAKE_C_STANDARD              99)
set(CMAKE_C_STANDARD_REQUIRED     ON)
set(CMAKE_C_EXTENSIONS            ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build Type" FORCE)
endif()

# Get Raylib.
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} REQUIRED)

# Cache project source files.
file(GLOB_RECURSE PROJECT_SOURCES
  "${CMAKE_SOURCE_DIR}/src/*.c"
  "${CMAKE_SOURCE_DIR}/src/*.h"
)

# Set executable name.
set(EXE_NAME out)

# Add executable target.
add_executable(${EXE_NAME} ${PROJECT_SOURCES})
target_include_directories(${EXE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/include/)
target_link_libraries(${EXE_NAME} raylib)

# Enable compiler warnings.
if(MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-W -Wall -Wpedantic -Werror -Wextra) 
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${EXE_NAME} "-framework IOKit")
    target_link_libraries(${EXE_NAME} "-framework Cocoa")
    target_link_libraries(${EXE_NAME} "-framework OpenGL")
endif()

# Output project summary.
message(STATUS "Project    : ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C Standard : ${CMAKE_C_STANDARD}")
message(STATUS "Build Type : ${CMAKE_BUILD_TYPE}")
