cmake_minimum_required(VERSION 3.23.1)

project(imgui)

add_library(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
	PROPERTIES
	CXX_STANDARD 11 
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(${PROJECT_NAME} PRIVATE
	imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_impl_glfw.cpp
	imgui_impl_vulkan.cpp
	imgui_tables.cpp
	imgui_widgets.cpp
	imconfig.h
	imgui.h
	imgui_impl_glfw.h
	imgui_impl_vulkan.h
	imgui_internal.h
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h
)

target_compile_options(${PROJECT_NAME} PRIVATE
	"$<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:"
	-Wall
	-Wshadow
	-Wextra
# -Werror
	-Wpedantic
	-pedantic-errors
	-Walloca
	-Wvla
	-Wzero-as-null-pointer-constant
	-fdiagnostics-color=always
	">"
	"$<$<AND:$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>,$<CONFIG:Debug>>:"
	-g
	# -fsanitize=address
	# -fsanitize=undefined
	# -fsanitize=bounds
	">"
	"$<$<AND:$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>,$<CONFIG:Release>>:"
	-Ofast
	">"
)

target_link_options(${PROJECT_NAME} PRIVATE
	"$<$<AND:$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>,$<CONFIG:Debug>>:"
	# -fsanitize=address
	# -fsanitize=undefined
	# -fsanitize=bounds
	">"
)

target_include_directories(${PROJECT_NAME} PUBLIC
	${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
	glfw
	${Vulkan_LIBRARIES}
)


