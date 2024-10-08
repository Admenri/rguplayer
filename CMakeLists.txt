## Main project file
cmake_minimum_required(VERSION 3.20.1)

#--------------------------------------------------------------------------------
# Project define
#--------------------------------------------------------------------------------

project(URGE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include_directories("${PROJECT_SOURCE_DIR}")

if (MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()

set(RGU_AUTOGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
include_directories("${RGU_AUTOGEN_DIR}")

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

#-------------------------------------------------------------------------
# Third party libraries
#-------------------------------------------------------------------------

## base 3rd
include_directories(base/third_party)
include_directories(third_party)

### Android compile configure
set(SDL_STATIC on)
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(SDL_STATIC off)
endif ()

## SDL
add_subdirectory(third_party/SDL)
include_directories(third_party/SDL/include)

## Bgfx Renderer
set(BGFX_CONFIG_MAX_VIEWS 16384)
set(BGFX_CONFIG_MAX_FRAME_BUFFERS 16384)
set(BGFX_CONFIG_MAX_DYNAMIC_VERTEX_BUFFERS 32768)
set(BGFX_CONFIG_MAX_DYNAMIC_INDEX_BUFFERS 16384)
add_subdirectory(third_party/bgfx.cmake)

## Physfs I/O
set(PHYSFS_BUILD_STATIC TRUE)
set(PHYSFS_BUILD_SHARED FALSE)
set(PHYSFS_BUILD_DOCS FALSE)
add_subdirectory(third_party/physfs)
include_directories(third_party/physfs/src)

## DAV1D
add_subdirectory(third_party/dav1d)
add_subdirectory(third_party/webm)
include_directories(third_party/webm)

## Vorbis OGG
add_subdirectory(third_party/ogg)
add_subdirectory(third_party/vorbis)

## Soloud
add_subdirectory(third_party/soloud)
include_directories(third_party/soloud/include)

## image loader (SDL_image)
add_subdirectory(third_party/image_loader)
include_directories(third_party/image_loader/include/SDL3_image)

## text render (SDL_ttf)
add_subdirectory(third_party/freetype)
include_directories(third_party/freetype/include)
add_subdirectory(third_party/text_render)
include_directories(third_party/text_render/include/SDL3_ttf)

## Zlib
add_subdirectory(third_party/zlib)
include_directories(third_party/zlib)

## IMGUI
add_subdirectory(third_party/imgui)

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

#--------------------------------------------------------------------------------
# Apply patch
#--------------------------------------------------------------------------------

add_subdirectory(patches)

#--------------------------------------------------------------------------------
# Components of Core
#--------------------------------------------------------------------------------

## FPSLimiter
add_subdirectory(components/fpslimiter)

## Filesystem
add_subdirectory(components/filesystem)

#-------------------------------------------------------------------------
# Engine source
#-------------------------------------------------------------------------

add_subdirectory(base)
add_subdirectory(ui)
add_subdirectory(renderer)
add_subdirectory(content)
add_subdirectory(binding)
add_subdirectory(app)
