# Main build file of rgu project
cmake_minimum_required(VERSION 3.18.0)

project(rgu_player VERSION 1.0.0)
include_directories("${PROJECT_SOURCE_DIR}")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_STANDARD 20)

#--------------------------------------------------------------------------------
# Platform and compiler check
#--------------------------------------------------------------------------------

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_compile_definitions(NOMINMAX)
  link_libraries(
    "shell32;ws2_32;iphlpapi;imagehlp;shlwapi;bcrypt"
    "setupapi;advapi32;uuid;version;oleaut32;ole32;imm32;winmm;user32;rpcrt4;usp10"
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  link_libraries("crypt")
endif()

#--------------------------------------------------------------------------------
# Third-party Libraries
#--------------------------------------------------------------------------------

## Third party include
include_directories(base/third_party)
include_directories(third_party)

## SDL
set(SDL_STATIC on)
add_subdirectory(third_party/SDL)

## SDL_image
set(BUILD_SHARED_LIBS off)
add_subdirectory(third_party/SDL_image)
include_directories(third_party/SDL_image/include/SDL3_image)

## SDL_ttf
set(SDL3TTF_VENDORED ON)
set(SDL3TTF_BUILD_SHARED_LIBS off)
add_subdirectory(third_party/SDL_ttf)
include_directories(third_party/SDL_ttf/include/SDL3_ttf)

## CRuby-Mri
find_package(Ruby REQUIRED)
add_library(mri_ruby UNKNOWN IMPORTED)
set(RUBY_INCLUDE_PATH "${Ruby_INCLUDE_DIRS}" "${Ruby_CONFIG_INCLUDE_DIR}")
set(RUBY_LIBRARY_PATH "${Ruby_LIBRARIES}")
message(${RUBY_INCLUDE_PATH} ${RUBY_LIBRARY_PATH})
set_target_properties(mri_ruby PROPERTIES
                      IMPORTED_LOCATION "${RUBY_LIBRARY_PATH}"
                      INTERFACE_INCLUDE_DIRECTORIES "${RUBY_INCLUDE_PATH}")

## Google ANGLE
include_directories(third_party/angle/include)

## Physfs
set(PHYSFS_BUILD_STATIC TRUE)
set(PHYSFS_BUILD_SHARED FALSE)
add_subdirectory(third_party/physfs)
include_directories(third_party/physfs/src)

## Zlib
add_subdirectory(third_party/zlib)
include_directories(third_party/zlib)

## Soloud
add_subdirectory(third_party/soloud)
include_directories(third_party/soloud/include)

#--------------------------------------------------------------------------------
# Components of Core
#--------------------------------------------------------------------------------

## FPSLimiter
add_subdirectory(components/fpslimiter)

## Filesystem
add_subdirectory(components/filesystem)

## SettingsMenu
add_subdirectory(components/settingsmenu)

#--------------------------------------------------------------------------------
# Auto-generated directory include path
#--------------------------------------------------------------------------------

find_package(Python3 COMPONENTS Interpreter)

if(Python3_FOUND)
    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
    message(STATUS "Found Python3: ${Python3_EXECUTABLE}")
else()
    message(FATAL_ERROR "Python3 not found.")
endif()

set(RGU_AUTOGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
include_directories("${RGU_AUTOGEN_DIR}")

#--------------------------------------------------------------------------------
# Add rgu projects
#--------------------------------------------------------------------------------

## Base components import
add_subdirectory(base)

## UI Wrapper
add_subdirectory(ui)

## GLESRenderer
add_subdirectory(renderer)

## Content compositor
add_subdirectory(content)

## Script binding
add_subdirectory(binding)

## Entry of the RGU player
add_subdirectory(app)
