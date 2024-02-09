# GLESRenderer of RGU Core
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# GLESContext command autogen
#--------------------------------------------------------------------------------

### GLES2.0 API ###

# Export header file import
execute_process(
  COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/buildtools/build_gles2_command_buffer.py" "--file=${PROJECT_SOURCE_DIR}/renderer/context/gles2_commands.txt" --header "--output=${RGU_AUTOGEN_DIR}/renderer/context/gles2_command_buffer_header_autogen.h"
  RESULT_VARIABLE PYTHON_SCRIPT_RESULT
  OUTPUT_QUIET
)

# Export header file import
execute_process(
  COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/buildtools/build_gles2_command_buffer.py" "--file=${PROJECT_SOURCE_DIR}/renderer/context/gles2_commands.txt" --body "--output=${RGU_AUTOGEN_DIR}/renderer/context/gles2_command_buffer_header_autogen.cc"
  RESULT_VARIABLE PYTHON_SCRIPT_RESULT
  OUTPUT_QUIET
)

if(NOT PYTHON_SCRIPT_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to generate gles2 header: ${PYTHON_SCRIPT_RESULT}")
endif()

#--------------------------------------------------------------------------------
# Auto-gen shader embed binary source
#--------------------------------------------------------------------------------

file(GLOB RENDERER_SHADER_FILES "${PROJECT_SOURCE_DIR}/renderer/shader/glsl/*")
foreach(SHADER_FILE ${RENDERER_SHADER_FILES})
  get_filename_component(FILE_EXTNAME ${SHADER_FILE} EXT)
  get_filename_component(FILE_NAME ${SHADER_FILE} NAME)
  if (FILE_EXTNAME STREQUAL ".vert" OR FILE_EXTNAME STREQUAL ".frag")
    execute_process(
      COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/buildtools/build_file_binary.py" "${SHADER_FILE}" -o "${RGU_AUTOGEN_DIR}/renderer/shader/glsl/${FILE_NAME}.xxd"
      RESULT_VARIABLE SHADER_GENERATE_RESULT
      OUTPUT_QUIET
    )
    if(NOT SHADER_GENERATE_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to generate shader embed file: ${SHADER_GENERATE_RESULT}")
    endif()
  endif()
endforeach()

#--------------------------------------------------------------------------------
# GLESRenderer components
#--------------------------------------------------------------------------------

set(CORE_RENDERER_SRC
  context/gles2_context.cc
  context/gles2_context.h
  meta/gles2meta.cc
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
)

add_library(core_renderer ${CORE_RENDERER_SRC})
target_link_libraries(core_renderer core_base SDL3::SDL3-static)
