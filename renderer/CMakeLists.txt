# GLESRenderer of RGU Core
cmake_minimum_required(VERSION 3.20.1)

#--------------------------------------------------------------------------------
# GLESContext command autogen
#--------------------------------------------------------------------------------

### GLES2.0 API ###

# Define the header and source output files
set(GLES2_HEADER_OUTPUT "${RGU_AUTOGEN_DIR}/renderer/context/gles_command_buffer_header_autogen.inc")
set(GLES2_SOURCE_OUTPUT "${RGU_AUTOGEN_DIR}/renderer/context/gles_command_buffer_define_autogen.inc")

# Add custom command to generate header file
add_custom_command(
  OUTPUT ${GLES2_HEADER_OUTPUT}
  COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/buildtools/build_gles2_command_buffer.py" "--file=${PROJECT_SOURCE_DIR}/renderer/context/gles2_commands.txt" --header "--output=${GLES2_HEADER_OUTPUT}"
  DEPENDS "${PROJECT_SOURCE_DIR}/renderer/context/gles2_commands.txt"
  COMMENT "Generating OGLES header file"
)

# Add custom command to generate source file
add_custom_command(
  OUTPUT ${GLES2_SOURCE_OUTPUT}
  COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/buildtools/build_gles2_command_buffer.py" "--file=${PROJECT_SOURCE_DIR}/renderer/context/gles2_commands.txt" --body "--output=${GLES2_SOURCE_OUTPUT}"
  DEPENDS "${PROJECT_SOURCE_DIR}/renderer/context/gles2_commands.txt"
  COMMENT "Generating OGLES source file"
)

# Add custom target that depends on the generated files
add_custom_target(generate_gles2_command_buffer ALL
  DEPENDS ${GLES2_HEADER_OUTPUT} ${GLES2_SOURCE_OUTPUT}
)

#--------------------------------------------------------------------------------
# Auto-gen shader embed binary source
#--------------------------------------------------------------------------------

# Define a custom command and target for each shader file
file(GLOB RENDERER_SHADER_FILES "${PROJECT_SOURCE_DIR}/renderer/shader/glsl/*")
foreach(SHADER_FILE ${RENDERER_SHADER_FILES})
  get_filename_component(FILE_EXTNAME ${SHADER_FILE} EXT)
  get_filename_component(FILE_NAME ${SHADER_FILE} NAME)
  if (FILE_EXTNAME STREQUAL ".vert" OR FILE_EXTNAME STREQUAL ".frag")
    set(SHADER_OUTPUT "${RGU_AUTOGEN_DIR}/renderer/shader/glsl/${FILE_NAME}.xxd")
    add_custom_command(
      OUTPUT ${SHADER_OUTPUT}
      COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/buildtools/build_file_binary.py" "${SHADER_FILE}" -o "${SHADER_OUTPUT}"
      DEPENDS "${SHADER_FILE}"
      COMMENT "Generating shader embed file for ${SHADER_FILE}"
    )
    list(APPEND SHADER_GENERATE_OUTPUTS ${SHADER_OUTPUT})
  endif()
endforeach()

# Add a custom target that depends on all generated shader files
add_custom_target(generate_shader_embed_files ALL
  DEPENDS ${SHADER_GENERATE_OUTPUTS}
)

#--------------------------------------------------------------------------------
# GLESRenderer components
#--------------------------------------------------------------------------------

set(CORE_RENDERER_SRC
  context/gles2_context.cc
  context/gles2_context.h
  draw/drawable.h
  meta/gles2meta.h
  quad/quad_array.h
  quad/quad_drawable.cc
  quad/quad_drawable.h
  shader/gles2_shaders.cc
  shader/gles2_shaders.h
  states/draw_states.cc
  states/draw_states.h
  thread/thread_manager.cc
  thread/thread_manager.h
  vertex/vertex_set.cc
  vertex/vertex_set.h
  ${GLES2_HEADER_OUTPUT}
  ${GLES2_SOURCE_OUTPUT}
  ${SHADER_GENERATE_OUTPUTS}
)

add_library(core_renderer ${CORE_RENDERER_SRC})
add_dependencies(core_renderer generate_gles2_command_buffer generate_shader_embed_files)
target_link_libraries(core_renderer PUBLIC core_base)
target_compile_options(core_renderer PRIVATE
  -Werror
  -Wno-float-conversion
)
