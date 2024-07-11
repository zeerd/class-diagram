cmake_minimum_required(VERSION 3.10)
project(class-diagram)

find_package(Clang REQUIRED)
find_package(LLVM REQUIRED CONFIG)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

execute_process(COMMAND git describe --always
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                OUTPUT_VARIABLE BUILD_ID
                OUTPUT_STRIP_TRAILING_WHITESPACE)
add_definitions(-DBUILD_ID="${BUILD_ID}")

file(GLOB SOURCES "src/*.cpp")

add_executable(
    class-diagram
    ${SOURCES}
)
target_include_directories(
    class-diagram PRIVATE
    ${CLANG_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/3rdparty/json/include
)
target_link_libraries(
    class-diagram PRIVATE
    ${LLVM_LIBS}
    ${CLANG_LIBS}
    clangFrontend clangTooling clangAST
)
target_compile_options(
    class-diagram PRIVATE
    -std=gnu++17 -frtti
)
install(
    TARGETS class-diagram
    DESTINATION bin
)
