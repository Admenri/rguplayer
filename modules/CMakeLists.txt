#  Main build file of rgu project
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Define export source
#--------------------------------------------------------------------------------

set(MODULES_SRC
  bitmap.cc
  bitmap.h
  disposable.h
)

add_library(core_modules ${MODULES_SRC})
target_link_libraries(core_modules SDL2::Core SDL2::Image)
