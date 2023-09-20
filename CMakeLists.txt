#  Main build file of rgu project
cmake_minimum_required(VERSION 3.18.0)

project(rgu)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${SRC_ROOT})

#--------------------------------------------------------------------------------
# Third party configure
#--------------------------------------------------------------------------------

if (WIN32)

set(SDL2_TTF_INCLUDE_DIR "${SRC_ROOT}/third_party/SDL2_ttf/include")
set(SDL2_IMAGE_INCLUDE_DIR "${SRC_ROOT}/third_party/SDL2_image/include")
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
set(SDL2_INCLUDE_DIR "${SRC_ROOT}/third_party/SDL2/include/x86")
set(SDL2_LIBRARY "${SRC_ROOT}/third_party/SDL2/lib/x86/SDL2-static.lib")
set(SDL2_TTF_LIBRARY "${SRC_ROOT}/third_party/SDL2_ttf/lib/x86/SDL2_ttf.lib")
link_libraries("${SRC_ROOT}/third_party/SDL2_ttf/lib/x86/freetype.lib")
set(SDL2_IMAGE_LIBRARY "${SRC_ROOT}/third_party/SDL2_image/lib/x86/SDL2_image-static.lib")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
set(SDL2_INCLUDE_DIR "${SRC_ROOT}/third_party/SDL2/include/x64")
set(SDL2_LIBRARY "${SRC_ROOT}/third_party/SDL2/lib/x64/SDL2-static.lib")
set(SDL2_TTF_LIBRARY "${SRC_ROOT}/third_party/SDL2_ttf/lib/x64/SDL2_ttf.lib")
link_libraries("${SRC_ROOT}/third_party/SDL2_ttf/lib/x64/freetype.lib")
set(SDL2_LIBRARY "${SRC_ROOT}/third_party/SDL2/lib/x64/SDL2-static.lib")
set(SDL2_IMAGE_LIBRARY "${SRC_ROOT}/third_party/SDL2_image/lib/x64/SDL2_image-static.lib")
endif()

link_libraries(
  "shell32;ws2_32;iphlpapi;imagehlp;shlwapi;bcrypt"
  "setupapi;advapi32;uuid;version;oleaut32;ole32;imm32;winmm;user32;rpcrt4;usp10"
)

elseif (LINUX)
message("No Implement")
elseif (MACOS)
message("No Implement")
elseif (ANDROID)
message("No Implement")
endif()

#--------------------------------------------------------------------------------
# Find and build package
#--------------------------------------------------------------------------------

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

#--------------------------------------------------------------------------------
# Add rgu projects
#--------------------------------------------------------------------------------

add_subdirectory(base)
add_subdirectory(gpu)
add_subdirectory(renderer)
add_subdirectory(app)
