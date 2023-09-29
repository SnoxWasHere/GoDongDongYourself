cmake_minimum_required (VERSION 3.5)
project(testproject2)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/)

add_compile_options(-Wno-narrowing)
add_compile_options(-Wno-c++20-extensions)

include_directories(src/header)

file(GLOB SOURCES "src/*.cpp")
 
add_executable(${PROJECT_NAME} ${SOURCES})