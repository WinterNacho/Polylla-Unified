cmake_minimum_required(VERSION 3.10)

project(Polylla LANGUAGES CXX C)

# Try to find CUDA (optional)
find_package(CUDA QUIET)
if(CUDA_FOUND)
    enable_language(CUDA)
    set(CUDA_AVAILABLE TRUE)
    message(STATUS "CUDA found - GPU acceleration will be available")
else()
    set(CUDA_AVAILABLE FALSE)
    message(STATUS "CUDA not found - building CPU-only version")
endif()

# Add subdirectories
add_subdirectory(external)
add_subdirectory(src)

# Include directories
include_directories(external src)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

if(CUDA_AVAILABLE)
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -O3")
    # Define preprocessor macro for conditional compilation
    add_definitions(-DCUDA_AVAILABLE)
endif()

# Specify main source file (CPU version uses unified main.cpp)
set(MAIN_SOURCE_FILE main.cpp)

# Create the main executable
add_executable(Polylla ${MAIN_SOURCE_FILE})

# Link libraries - always link malloc_count for both versions
target_link_libraries(Polylla PUBLIC meshfiles malloccountfiles)

if(CUDA_AVAILABLE)
    # Link CUDA runtime for GPU version
    target_link_libraries(Polylla PUBLIC ${CUDA_LIBRARIES})
    message(STATUS "Linking with CUDA libraries")
endif()

# Set linker language
set_target_properties(Polylla PROPERTIES LINKER_LANGUAGE CXX)
