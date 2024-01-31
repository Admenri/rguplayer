# Mri binding components
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Mri - Matz's Ruby Interpreter components
#--------------------------------------------------------------------------------

set(BINDING_MRI_SRC
  init_corefile.cc
  init_corefile.h
  init_table.cc
  init_table.h
  init_utility.cc
  init_utility.h
  init_viewport.cc
  init_viewport.h

  mri_template.h
  mri_main.cc
  mri_main.h
  mri_util.cc
  mri_util.h
)

add_library(binding_mri ${BINDING_MRI_SRC})
target_link_libraries(binding_mri core_base core_renderer core_content mri_ruby zlibstatic)
