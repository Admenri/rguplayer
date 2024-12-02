# Mri binding components
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Mri - Matz's Ruby Interpreter components
#--------------------------------------------------------------------------------

set(BINDING_MRI_SRC
  init_aomdecode.cc
  init_aomdecode.h
  init_audio.cc
  init_audio.h
  init_bitmap.cc
  init_bitmap.h
  init_corefile.cc
  init_corefile.h
  init_font.cc
  init_font.h
  init_gamepad.cc
  init_gamepad.h
  init_graphics.cc
  init_graphics.h
  init_input.cc
  init_input.h
  init_plane.cc
  init_plane.h
  init_sprite.cc
  init_sprite.h
  init_table.cc
  init_table.h
  init_tilemap.cc
  init_tilemap.h
  init_tilemap2.cc
  init_tilemap2.h
  init_touch.cc
  init_touch.h
  init_utility.cc
  init_utility.h
  init_viewport.cc
  init_viewport.h
  init_window.cc
  init_window.h
  init_window2.cc
  init_window2.h

  init_rgu.cc
  init_rgu.h
  init_mouse.cc
  init_mouse.h
  init_geometry.cc
  init_geometry.h
  init_shader.cc
  init_shader.h
  init_http.cc
  init_http.h

  mri_template.h
  mri_main.cc
  mri_main.h
  mri_util.cc
  mri_util.h

  zlib.c

  net/net_http.cc
  net/net_http.h

  ruby_prof/rp_allocation.c
  ruby_prof/rp_call_tree.c
  ruby_prof/rp_call_trees.c
  ruby_prof/rp_measure_allocations.c
  ruby_prof/rp_measure_memory.c
  ruby_prof/rp_measure_process_time.c
  ruby_prof/rp_measure_wall_time.c
  ruby_prof/rp_measurement.c
  ruby_prof/rp_method.c
  ruby_prof/rp_profile.c
  ruby_prof/rp_stack.c
  ruby_prof/rp_thread.c
  ruby_prof/ruby_prof.c
)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
set(STEAMWORKS_DIR ${PROJECT_SOURCE_DIR}/steamworks)
if(EXISTS ${STEAMWORKS_DIR})
  add_library(external-steamworks UNKNOWN IMPORTED)

  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(STEAMWORKS_LIB_PATH "steam_api.lib")  
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(STEAMWORKS_LIB_PATH "win64/steam_api64.lib")
    endif()
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(STEAMWORKS_LIB_PATH "linux32/libsteam_api.so")  
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(STEAMWORKS_LIB_PATH "linux64/libsteam_api.so")
    endif()
  endif()
  set_target_properties(external-steamworks PROPERTIES
                        IMPORTED_LOCATION "${STEAMWORKS_DIR}/bin/${STEAMWORKS_LIB_PATH}"
                        INTERFACE_INCLUDE_DIRECTORIES "${STEAMWORKS_DIR}/public")

  set(BINDING_MRI_SRC
    ${BINDING_MRI_SRC}

    init_steamworks.cc
    init_steamworks.h
  )
endif()
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
set(LIBFFI_DIR ${PROJECT_SOURCE_DIR}/third_party/libffi)
if(EXISTS ${LIBFFI_DIR})
  add_library(external-libffi UNKNOWN IMPORTED)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set_target_properties(external-libffi PROPERTIES
                          IMPORTED_LOCATION "${LIBFFI_DIR}/lib/libffi.lib"
                          INTERFACE_INCLUDE_DIRECTORIES "${LIBFFI_DIR}/include")
  else()
    message(error)
  endif()

  set(BINDING_MRI_SRC
    ${BINDING_MRI_SRC}

    fiddle/closure.c
    fiddle/closure.h
    fiddle/conversions.c
    fiddle/conversions.h
    fiddle/fiddle.c
    fiddle/fiddle.h
    fiddle/function.c
    fiddle/function.h
    fiddle/handle.c
    fiddle/memory_view.c
    fiddle/pinned.c
    fiddle/pointer.c
  )
endif()
endif()

add_library(binding_mri ${BINDING_MRI_SRC})
target_link_libraries(binding_mri PUBLIC core_base core_renderer core_content core_ui mri_ruby zlibstatic lurl_parser)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
if(EXISTS ${STEAMWORKS_DIR})
  target_compile_definitions(binding_mri PRIVATE HAS_STEAMWORKS_SUPPORT=1)
  target_link_libraries(binding_mri PRIVATE external-steamworks)
endif()
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
if(EXISTS ${LIBFFI_DIR})
  target_compile_definitions(binding_mri PRIVATE HAS_LIBFFI_SUPPORT=1)
  target_link_libraries(binding_mri PRIVATE external-libffi)
  target_include_directories(binding_mri PRIVATE fiddle)
endif()
endif()