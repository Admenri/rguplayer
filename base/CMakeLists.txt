# base components
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Define base source
#--------------------------------------------------------------------------------

set(BASE_SRC
  debug/debugwriter.h
  exceptions/exception.h
  math/math.cc
  math/math.h
)

add_library(core_base ${BASE_SRC})
target_link_libraries(core_base SDL2::Core)
