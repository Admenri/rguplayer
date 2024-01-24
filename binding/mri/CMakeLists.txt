# Mri binding components
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Mri - Matz's Ruby Interpreter components
#--------------------------------------------------------------------------------

set(BINDING_MRI_SRC
  mri_main.cc
  mri_main.h
)

add_library(binding_mri ${BINDING_MRI_SRC})
target_link_libraries(binding_mri core_base mri_ruby)