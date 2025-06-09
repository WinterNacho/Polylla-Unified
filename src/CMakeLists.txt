# Specify exact files for each Polylla version

# CPU version files (header-only implementation)
set(POLYLLA_CPU_FILES
    polylla.hpp
    triangulation.hpp
    polylla_gpu.hpp
)

# GPU version files (CUDA implementation)
set(POLYLLA_GPU_FILES
    polylla.cu
    triangulation.cu
    kernel.cu
    polylla_gpu.hpp
)

# Choose which version to compile based on CUDA availability
if(CUDA_AVAILABLE)
    # Use GPU version with CUDA
    add_library(meshfiles STATIC ${POLYLLA_CPU_FILES} ${POLYLLA_GPU_FILES})
    set_target_properties(meshfiles PROPERTIES LINKER_LANGUAGE CXX)
    
    # Set CUDA properties for .cu files
    set_property(SOURCE polylla.cu triangulation.cu kernel.cu PROPERTY LANGUAGE CUDA)
    
    # Add include directories for external dependencies (only needed for CUDA version)
    target_include_directories(meshfiles PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../external
    )
    
    message(STATUS "Building with GPU version (CUDA files)")
else()
    # Use CPU version only
    add_library(meshfiles STATIC ${POLYLLA_CPU_FILES})
    set_target_properties(meshfiles PROPERTIES LINKER_LANGUAGE CXX)
    message(STATUS "Building with CPU version (header-only files)")
endif()
