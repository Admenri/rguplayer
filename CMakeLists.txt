# Main build file of rgu project
cmake_minimum_required(VERSION 3.20.1)

project(rgu_player)
set(CMAKE_CXX_STANDARD 17)
include_directories("${PROJECT_SOURCE_DIR}")

if (MSVC)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()

#--------------------------------------------------------------------------------
# Platform and compiler check
#--------------------------------------------------------------------------------

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_compile_definitions(NOMINMAX)
  link_libraries(
    "shell32;ws2_32;iphlpapi;imagehlp;shlwapi;bcrypt"
    "setupapi;advapi32;uuid;version;oleaut32;ole32;imm32;winmm;user32;rpcrt4;usp10"
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  link_libraries(crypt)
endif ()

#--------------------------------------------------------------------------------
# Auto-generated directory include path
#--------------------------------------------------------------------------------

find_package(Python3 COMPONENTS Interpreter)

if (Python3_FOUND)
    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
    message(STATUS "Found Python3: ${Python3_EXECUTABLE}")
else ()
    message(FATAL_ERROR "Python3 not found.")
endif ()

set(RGU_AUTOGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
include_directories("${RGU_AUTOGEN_DIR}")

#--------------------------------------------------------------------------------
# Third-party Libraries
#--------------------------------------------------------------------------------

## Third party include
include_directories(base/third_party)
include_directories(third_party)

## Google ANGLE Headers
include_directories(third_party/angle/include)

### Android compile configure
set(SDL_STATIC on)
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
set(SDL_STATIC off)
endif ()

## SDL
add_subdirectory(third_party/SDL)
include_directories(third_party/SDL/include)
include_directories(third_party/SDL/include/SDL3)

## image loader (SDL_image)
add_subdirectory(third_party/image_loader)
include_directories(third_party/image_loader/include/SDL3_image)

## text render (SDL_ttf)
add_subdirectory(third_party/freetype)
include_directories(third_party/freetype/include)
add_subdirectory(third_party/text_render)
include_directories(third_party/text_render/include/SDL3_ttf)

## CRuby-Mri
## TODO: Enhance ruby build ins
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
  add_subdirectory(third_party/cmake-ruby)
  include_directories(third_party/cmake-ruby/include)
else ()
  add_library(mri_ruby UNKNOWN IMPORTED)
  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(RUBY_INCLUDE_PATH
        "${PROJECT_SOURCE_DIR}/third_party/ruby/include/"
        "${PROJECT_SOURCE_DIR}/third_party/ruby/config/x86/"
      )
      set(RUBY_LIBRARY_PATH
        "${PROJECT_SOURCE_DIR}/third_party/ruby/lib/x86/vcruntime140-ruby320-static.lib"
      )
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(RUBY_INCLUDE_PATH
        "${PROJECT_SOURCE_DIR}/third_party/ruby/include/"
        "${PROJECT_SOURCE_DIR}/third_party/ruby/config/x64/"
      )
      set(RUBY_LIBRARY_PATH
        "${PROJECT_SOURCE_DIR}/third_party/ruby/lib/x64/x64-vcruntime140-ruby320-static.lib"
      )
    else ()
      message("No Implement")
    endif ()
  else ()
    find_package(Ruby REQUIRED)
    set(RUBY_INCLUDE_PATH "${Ruby_INCLUDE_DIRS}" "${Ruby_CONFIG_INCLUDE_DIR}")
    set(RUBY_LIBRARY_PATH "${Ruby_LIBRARIES}")
    message(${RUBY_INCLUDE_PATH} ${RUBY_LIBRARY_PATH})
  endif ()
  set_target_properties(mri_ruby PROPERTIES
                        IMPORTED_LOCATION "${RUBY_LIBRARY_PATH}"
                        INTERFACE_INCLUDE_DIRECTORIES "${RUBY_INCLUDE_PATH}")
endif ()

## Physfs I/O
set(PHYSFS_BUILD_STATIC TRUE)
set(PHYSFS_BUILD_SHARED FALSE)
set(PHYSFS_BUILD_DOCS FALSE)
add_subdirectory(third_party/physfs)
include_directories(third_party/physfs/src)

## Zlib
add_subdirectory(third_party/zlib)
include_directories(third_party/zlib)

## AOM/Video Decode
include_directories("${CMAKE_CURRENT_BINARY_DIR}/third_party/aom")
add_subdirectory(third_party/aom)
include_directories(third_party/aom)
include_directories(third_party/aom/third_party/libwebm)
add_subdirectory(third_party/av1player)

## IMGUI
add_subdirectory(third_party/imgui)
include_directories(third_party/imgui)

## Audio
set(LIBTYPE STATIC)
include_directories(third_party/sound_decoder/src)
add_subdirectory(third_party/openal-soft)
add_subdirectory(third_party/sound_decoder)
add_subdirectory(third_party/ogg)
add_subdirectory(third_party/vorbis)

## Network
add_subdirectory(third_party/boost_asio)
include_directories(third_party/websocketpp)

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
