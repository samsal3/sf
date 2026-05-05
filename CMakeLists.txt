cmake_minimum_required(VERSION 3.1...3.29)

project(sf)

add_library(${PROJECT_NAME})

find_package(Vulkan REQUIRED)

set_target_properties(${PROJECT_NAME}
PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_compile_options(${PROJECT_NAME} PRIVATE
  "$<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>:"
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
  "$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Debug>>:"
  -g
#    -fsanitize=address
#    -fsanitize=undefined
#    -fsanitize=bounds
  -Wuninitialized
  ">"
  "$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Release>>:"
  -Wuninitialized
  -Ofast
  ">"
)

target_link_options(${PROJECT_NAME} PRIVATE
  "$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Debug>>:"
#    -fsanitize=address
#    -fsanitize=undefined
#    -fsanitize=bounds
  ">"
)


add_subdirectory(source)
add_subdirectory(vendor)
add_subdirectory(glsl)

target_include_directories(${PROJECT_NAME} PUBLIC 
  ${Vulkan_INCLUDE_DIRS}
  source
)

target_link_libraries(${PROJECT_NAME} 
  PUBLIC 
  cglm_headers
  glfw
  imgui
  stb
  PRIVATE
  ${Vulkan_LIBRARIES}
)

target_compile_definitions("${PROJECT_NAME}" 
  PRIVATE 
  "$<$<CONFIG:Debug>:SF_VULKAN_ENABLE_VALIDATOIN_LAYERS>"
)

add_executable("${PROJECT_NAME}_main" source/main.c)

set_target_properties("${PROJECT_NAME}_main"
  PROPERTIES 
  C_STANDARD       99
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS      OFF
)

target_compile_options("${PROJECT_NAME}_main"  PRIVATE
  "$<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>:"
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
  -Wno-declaration-after-statement #cglm
  ">"
  "$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Debug>>:"
  -g
#  -fsanitize=address
#  -fsanitize=undefined
#  -fsanitize=bounds
  ">"
  "$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Release>>:"
  -Ofast
  ">"
  "$<$<COMPILE_LANG_AND_ID:C,MSVC>:"
  /W4
  ">"
)


target_link_options("${PROJECT_NAME}_main" PRIVATE
  "$<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>:"
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
  -Wno-declaration-after-statement #cglm
  ">"
  "$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Debug>>:"
#    -fsanitize=address
#    -fsanitize=undefined
#    -fsanitize=bounds
  ">"
  "$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Release>>:"
  -Ofast
  ">"
)

file(COPY "resources" DESTINATION "${CMAKE_BINARY_DIR}")

target_link_libraries("${PROJECT_NAME}_main" PRIVATE ${PROJECT_NAME})
