# Main entry
cmake_minimum_required(VERSION 3.18.0)

add_executable(rgu_player rgu_main.cc resources/app.rc)
target_link_libraries(rgu_player core_renderer core_base core_content SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf mri_ruby physfs-static)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
set(COPY_SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/angle/bin/x86)
else()
set(COPY_SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/angle/bin/x64)
endif()
set(COPY_DEST_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(TARGET rgu_player POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${COPY_SOURCE_DIR} ${COPY_DEST_DIR}
)
