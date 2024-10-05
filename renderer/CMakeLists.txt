# Renderer components
cmake_minimum_required(VERSION 3.20.1)

#--------------------------------------------------------------------------------
# Renderer source
#--------------------------------------------------------------------------------

set(CORE_RENDERER_SRC
	device/render_device.cc
	device/render_device.h
  drawable/input_layout.cc
  drawable/input_layout.h
	drawable/quad_array.cc
	drawable/quad_array.h
	drawable/quad_drawable.cc
	drawable/quad_drawable.h
	pipeline/render_pipeline.cc
	pipeline/render_pipeline.h
)

#--------------------------------------------------------------------------------
# Shader compile
#--------------------------------------------------------------------------------

set(SHADER_GEN_OUTPUT ${RGU_AUTOGEN_DIR}/renderer/pipeline/shaders)
function(renderer_shader_compile shader_name shader_type)
	set(SHADER_VARYING_DEFINE_FILE ${PROJECT_SOURCE_DIR}/renderer/pipeline/shaders/common/varying.def.sc)
	bgfx_compile_shaders(
		TYPE ${shader_type}
		SHADERS pipeline/shaders/${shader_name}.sc
		VARYING_DEF ${SHADER_VARYING_DEFINE_FILE}
		OUTPUT_DIR ${SHADER_GEN_OUTPUT}
		AS_HEADERS
	)
	set(RENDERER_SHADERS_SOURCE
		${RENDERER_SHADERS_SOURCE}
		pipeline/shaders/${shader_name}.sc
	PARENT_SCOPE)
	set(GEN_SHADERS_SOURCE
		${GEN_SHADERS_SOURCE}
		${shader_name}.sc
	PARENT_SCOPE)
endfunction()

renderer_shader_compile(base_vert VERTEX)
renderer_shader_compile(base_frag FRAGMENT)
renderer_shader_compile(texblt_frag FRAGMENT)
renderer_shader_compile(basecolor_vert VERTEX)
renderer_shader_compile(basecolor_frag FRAGMENT)
renderer_shader_compile(hue_frag FRAGMENT)
renderer_shader_compile(viewport_frag FRAGMENT)
renderer_shader_compile(transform_vert VERTEX)
renderer_shader_compile(alphasprite_frag FRAGMENT)
renderer_shader_compile(sprite_frag FRAGMENT)
renderer_shader_compile(plane_frag FRAGMENT)
renderer_shader_compile(flatflash_frag FRAGMENT)
renderer_shader_compile(basealpha_vert VERTEX)
renderer_shader_compile(basealpha_frag FRAGMENT)
renderer_shader_compile(tilemap_vert VERTEX)
renderer_shader_compile(tilemap2_vert VERTEX)
renderer_shader_compile(alphatrans_frag FRAGMENT)
renderer_shader_compile(vaguetrans_frag FRAGMENT)

string(REPLACE ";" " " SHADER_LIST "${GEN_SHADERS_SOURCE}")
set(SHADER_OUTPUT_FILE "${RGU_AUTOGEN_DIR}/renderer/pipeline/shaders/shader_set_autogen.h")

add_custom_command(
  OUTPUT ${SHADER_OUTPUT_FILE}
  COMMAND ${CMAKE_COMMAND} -E echo "Generating shader include header file..."
  COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/buildtools/generate_shader_include.py ${SHADER_OUTPUT_FILE} ${SHADER_LIST}
  DEPENDS ${PROJECT_SOURCE_DIR}/buildtools/generate_shader_include.py
  COMMENT "Generating C++ include header with shader list: ${GEN_SHADERS_SOURCE}"
)

add_library(engine_renderer ${CORE_RENDERER_SRC} ${RENDERER_SHADERS_SOURCE} ${SHADER_OUTPUT_FILE})
target_link_libraries(engine_renderer PUBLIC engine_base engine_ui bx bgfx)
target_include_directories(engine_renderer PRIVATE ${SHADER_GEN_OUTPUT})
