# Main build file of rgu project
cmake_minimum_required(VERSION 3.18.0)

project(rgu_player VERSION 1.0.0)
include_directories("${PROJECT_SOURCE_DIR}")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_STANDARD 17)

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
# Third-party Libraries
#--------------------------------------------------------------------------------

## Third party include
include_directories(base/third_party)
include_directories(third_party)

### Android compile configure
set(SDL_STATIC on)
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
set(SDL_STATIC off)
endif()

if(SDL_STATIC)
  set(BUILD_SHARED_LIBS off)
else()
  set(BUILD_SHARED_LIBS on)
endif()

if(SDL_STATIC)
  set(SDL3TTF_BUILD_SHARED_LIBS off)
else()
  set(SDL3TTF_BUILD_SHARED_LIBS on)
endif()

if(SDL_STATIC)
  set(SDL3NET_BUILD_SHARED_LIBS off)
else()
  set(SDL3NET_BUILD_SHARED_LIBS on)
endif()

## SDL
add_subdirectory(third_party/SDL)

## SDL_image
set(SDL3IMAGE_VENDORED off)
add_subdirectory(third_party/SDL_image)
include_directories(third_party/SDL_image/include/SDL3_image)

## SDL_ttf
set(SDL3TTF_VENDORED ON)
add_subdirectory(third_party/SDL_ttf)
include_directories(third_party/SDL_ttf/include/SDL3_ttf)

## CRuby-Mri
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  add_subdirectory(third_party/cmake-ruby)
  include_directories(third_party/cmake-ruby/include)
else()
  add_library(mri_ruby UNKNOWN IMPORTED)
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(RUBY_INCLUDE_PATH
        "${PROJECT_SOURCE_DIR}/third_party/ruby/include/"
        "${PROJECT_SOURCE_DIR}/third_party/ruby/config/x86/"
      )
      set(RUBY_LIBRARY_PATH
        "${PROJECT_SOURCE_DIR}/third_party/ruby/lib/x86/vcruntime140-ruby320-static.lib"
      )
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(RUBY_INCLUDE_PATH
        "${PROJECT_SOURCE_DIR}/third_party/ruby/include/"
        "${PROJECT_SOURCE_DIR}/third_party/ruby/config/x64/"
      )
      set(RUBY_LIBRARY_PATH
        "${PROJECT_SOURCE_DIR}/third_party/ruby/lib/x64/x64-vcruntime140-ruby320-static.lib"
      )
    else()
      message("No Implement")
    endif()
  else()
    find_package(Ruby REQUIRED)
    set(RUBY_INCLUDE_PATH "${Ruby_INCLUDE_DIRS}" "${Ruby_CONFIG_INCLUDE_DIR}")
    set(RUBY_LIBRARY_PATH "${Ruby_LIBRARIES}")
    message(${RUBY_INCLUDE_PATH} ${RUBY_LIBRARY_PATH})
  endif()
  set_target_properties(mri_ruby PROPERTIES
                        IMPORTED_LOCATION "${RUBY_LIBRARY_PATH}"
                        INTERFACE_INCLUDE_DIRECTORIES "${RUBY_INCLUDE_PATH}")
endif()

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

## IMGUI
add_subdirectory(third_party/imgui)
include_directories(third_party/imgui)

#--------------------------------------------------------------------------------
# Components of Core
#--------------------------------------------------------------------------------

## FPSLimiter
add_subdirectory(components/fpslimiter)

## Filesystem
add_subdirectory(components/filesystem)

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
