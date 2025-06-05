cmake_minimum_required(VERSION 3.24)
project(yalsat VERSION 1.0.0 LANGUAGES C CXX CUDA)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(src_dir "${PROJECT_SOURCE_DIR}/src")
set(include_dir "${PROJECT_SOURCE_DIR}/include")
set(cpu_src "${PROJECT_SOURCE_DIR}/src/cpu")


# Source files
set(YALS_SOURCES
    ${cpu_src}/yals.c
    ${cpu_src}/config.c
)

set(MAIN_SOURCES
    ${src_dir}/main.c
)

# Create the YALS library
add_library(yals STATIC ${YALS_SOURCES})
target_include_directories(yals PUBLIC ${include_dir})

# Create the main executable
add_executable(yalsat ${MAIN_SOURCES})
target_link_libraries(yalsat PRIVATE yals)

# parallel version with pthread and -DPALSAT
add_executable(palsat ${MAIN_SOURCES})
target_compile_definitions(palsat PRIVATE -DPALSAT)
target_link_libraries(palsat PRIVATE yals pthread)

# Enable testing
# enable_testing()

# # Add version information
# configure_file(
#     ${CMAKE_CURRENT_SOURCE_DIR}/VERSION
#     ${CMAKE_CURRENT_BINARY_DIR}/version.h
#     @ONLY
# )

########################################################
# CUDA version
########################################################

if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES 80)
endif()

# Add the cuco and rmm dependencies
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/RAPIDS.cmake)
    file(DOWNLOAD https://raw.githubusercontent.com/rapidsai/rapids-cmake/branch-25.04/RAPIDS.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/RAPIDS.cmake)
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/RAPIDS.cmake)
include(rapids-cmake)
include(rapids-cpm)
include(rapids-cuda)
include(rapids-export)
include(rapids-find)
# include(rapids-rmm)
rapids_cpm_init()

include("${rapids-cmake-dir}/cpm/rmm.cmake")
rapids_cpm_rmm()

set(gpu_src "${PROJECT_SOURCE_DIR}/gpu/src")
set(gpu_include "${PROJECT_SOURCE_DIR}/include")

file(GLOB gpu_src "${gpu_src}/*.cu")