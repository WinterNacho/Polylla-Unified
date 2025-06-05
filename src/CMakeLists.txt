# Specify exact files for each Polylla version

# CPU version files (header-only implementation)
set(POLYLLA_CPU_FILES
    polylla.hpp
    triangulation.hpp
)

# GPU version files (CUDA implementation)
set(POLYLLA_GPU_FILES
    polylla.cu
    triangulation.cu
    kernel.cu
    polylla_gpu.hpp  # Header for GPU class declarations
)

# Choose which version to compile based on CUDA availability
if(CUDA_AVAILABLE)
    # Use GPU version with CUDA
    add_library(meshfiles STATIC ${POLYLLA_CPU_FILES} ${POLYLLA_GPU_FILES})
    set_target_properties(meshfiles PROPERTIES LINKER_LANGUAGE CXX)
    message(STATUS "Building with GPU version (CUDA files)")
else()
    # Use CPU version only
    # For header-only libraries, we need at least one source file
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "// Dummy file for header-only library\n")
    add_library(meshfiles STATIC ${POLYLLA_CPU_FILES} ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
    set_target_properties(meshfiles PROPERTIES LINKER_LANGUAGE CXX)
    message(STATUS "Building with CPU version (header-only files)")
endif()
