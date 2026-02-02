cmake_minimum_required(VERSION 3.23.1)

project(stb)

add_library(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
   PROPERTIES
   C_STANDARD 99 
   C_STANDARD_REQUIRED ON
   C_EXTENSIONS OFF
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(${PROJECT_NAME} PRIVATE
   stb_image.c
   stb_image.h
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
#     -fsanitize=address
#     -fsanitize=undefined
#     -fsanitize=bounds
   ">"
   "$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Release>>:"
      -Ofast
   ">"
)

target_link_options(${PROJECT_NAME} PRIVATE
   "$<$<AND:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<CONFIG:Debug>>:"
#     -fsanitize=address
#     -fsanitize=undefined
#     -fsanitize=bounds
  ">"
)


