set(SRCS
	gl/flextGL.c gl/flextGL.h
	gl/GLRenderer.cpp gl/GLRenderer.h
	gl/GLProfiler.cpp
	gl/GLState.h
	gl/GLMapping.h
	gl/GLShader.cpp
	gl/GLHelper.cpp gl/GLHelper.h
	Buffer.cpp Buffer.h
	Camera.cpp Camera.h
	Cubemap.cpp Cubemap.h
	FrameBuffer.cpp FrameBuffer.h
	FrameBufferConfig.cpp FrameBufferConfig.h
	GBuffer.cpp GBuffer.h
	Mesh.cpp Mesh.h
	MeshLUAFunctions.cpp MeshLUAFunctions.h
	MeshPool.cpp MeshPool.h
	Renderer.cpp Renderer.h
	RenderBuffer.cpp RenderBuffer.h
	Shader.cpp Shader.h
	ShapeBuilder.cpp ShapeBuilder.h
	ShaderManager.cpp ShaderManager.h
	ScopedLineWidth.h ScopedLineWidth.cpp
	ScopedViewPort.h
	ScopedFrameBuffer.h
	ScopedState.h
	ScopedBlendMode.h ScopedBlendMode.cpp
	ScopedScissor.h
	ScopedPolygonMode.h
	StencilConfig.h StencilConfig.cpp
	Texture.cpp Texture.h
	TextureConfig.cpp TextureConfig.h
	TexturePool.cpp TexturePool.h
	Types.h
	UniformBuffer.cpp UniformBuffer.h
	Version.h
	WindowedApp.cpp WindowedApp.h
)
set(SRCS_SHADERS
	shaders/_fog.vert shaders/_fog.frag
	shaders/_fullscreen.vert
	shaders/_shadowmap.vert shaders/_shadowmap.frag
)
set(FILES
	shared/keybindings.cfg

	client/mesh/chr_knight.fbx
	client/mesh/chr_knight.png
	client/mesh/chr_knight.lua
)

#set_source_files_properties(${SRCS_SHADERS} PROPERTIES LANGUAGE C)
set(LIB video)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL)
engine_add_module(TARGET ${LIB} SRCS ${SRCS} ${SRCS_SHADERS} FILES ${FILES} DEPENDENCIES image util assimp commonlua)

find_program(PYTHON3_EXECUTABLE NAMES python3)
add_custom_target(update-videobindings)
if (PYTHON3_EXECUTABLE)
	add_custom_command(TARGET update-videobindings
		COMMAND ${PYTHON3_EXECUTABLE} flextGLgen.py -T sdl -D ${CMAKE_CURRENT_SOURCE_DIR}/gl profiles/gl33.txt
		DEPENDS profiles/gl33.txt ${ROOT_DIR}/tools/flextGL/flextGLgen.py
		WORKING_DIRECTORY ${ROOT_DIR}/tools/flextGL
	)
endif()

set(TEST_SRCS
	tests/AbstractGLTest.h
	tests/ShaderTest.cpp
	tests/CameraTest.cpp
	tests/MeshTest.cpp
	tests/RendererTest.cpp
)

gtest_suite_sources(tests ${TEST_SRC})
gtest_suite_deps(tests ${LIB})

gtest_suite_begin(tests-${LIB} TEMPLATE ${ROOT_DIR}/src/modules/core/tests/main.cpp.in)
gtest_suite_sources(tests-${LIB} ${TEST_SRCS} ../core/tests/AbstractTest.cpp)
gtest_suite_deps(tests-${LIB} ${LIB})
gtest_suite_end(tests-${LIB})
