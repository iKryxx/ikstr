cmake_minimum_required(VERSION 3.26)
project(ikstr C)

# Use the C11 standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Collect source and header files
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/inc/*.h")

# Create library
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
target_compile_definitions(${PROJECT_NAME} PRIVATE IKSTR_EXPORTS)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/inc)

# Compiler warnings / optimizations
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif ()

add_executable(${PROJECT_NAME}_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/main.c
)

target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}_test PRIVATE ${CMAKE_SOURCE_DIR}/inc)

if (MSVC)
    target_compile_options(${PROJECT_NAME}_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME}_test PRIVATE -Wall -Wextra -Wpedantic -O3)
endif()

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
set_target_properties(${PROJECT_NAME}_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
