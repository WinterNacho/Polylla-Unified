cmake_minimum_required(VERSION 3.10)

project(Polylla LANGUAGES CXX C)

# Try to find CUDA (optional) - Modern CMake approach
include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    set(CUDA_AVAILABLE TRUE)
    set(CMAKE_CUDA_STANDARD 17)
    # CUDA architectures will be set per target if needed
    message(STATUS "CUDA found - GPU acceleration will be available")
else()
    set(CUDA_AVAILABLE FALSE)
    message(STATUS "CUDA not found - building CPU-only version")
endif()

# Add subdirectories
if(CUDA_AVAILABLE)
    # Include external libraries only when CUDA is available
    add_subdirectory(external)
    include_directories(external)
    set_target_properties(malloccountfiles PROPERTIES LINKER_LANGUAGE C)
endif()

add_subdirectory(src)
include_directories(src)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

if(CUDA_AVAILABLE)
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -O3")
    # Define preprocessor macro for conditional compilation
    add_definitions(-DCUDA_AVAILABLE)
endif()

# Create the main executable (using main.cpp - will be updated in Phase 2)
add_executable(Polylla main.cpp)

# Link libraries
target_link_libraries(Polylla PUBLIC meshfiles)

if(CUDA_AVAILABLE)
    # Link external libraries only when CUDA is available
    target_link_libraries(Polylla PUBLIC malloccountfiles)
    message(STATUS "Linking with CUDA libraries")
endif()

# Set linker language
if(CUDA_AVAILABLE)
    set_target_properties(Polylla PROPERTIES LINKER_LANGUAGE CUDA)
else()
    set_target_properties(Polylla PROPERTIES LINKER_LANGUAGE CXX)
endif()
