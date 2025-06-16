# CPU version files (always compiled)
set(POLYLLA_CPU_FILES
    polylla.hpp
    triangulation.hpp
    measure.hpp
    m_edge_ratio.hpp
)

# GPU version files (compiled only when CUDA is available)
set(POLYLLA_GPU_FILES
    GPolylla/polylla.cu
    GPolylla/triangulation.cu
    GPolylla/kernel.cu
)

# Choose which version to compile based on CUDA availability
if(CUDA_AVAILABLE)
    # Use both CPU and GPU versions when CUDA is available
    add_library(meshfiles STATIC ${POLYLLA_CPU_FILES} ${POLYLLA_GPU_FILES})
    
    # Set CUDA properties for .cu files only
    set_property(SOURCE 
        GPolylla/polylla.cu 
        GPolylla/triangulation.cu 
        GPolylla/kernel.cu 
        PROPERTY LANGUAGE CUDA
    )
    
    # Set CUDA architectures for meshfiles target (use default if not specified)
    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
        set_target_properties(meshfiles PROPERTIES 
            CUDA_ARCHITECTURES "70;75;80;86"
        )
    endif()
    
    # Add include directories for GPU version
    target_include_directories(meshfiles PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../external
        ${CMAKE_CURRENT_SOURCE_DIR}/GPolylla
    )
    
    message(STATUS "Building with GPU support (CPU + CUDA files)")
else()
    # Use CPU version only when CUDA is not available
    add_library(meshfiles STATIC ${POLYLLA_CPU_FILES})
    message(STATUS "Building with CPU version only (header-only files)")
endif()

# Set linker language
if(CUDA_AVAILABLE)
    set_target_properties(meshfiles PROPERTIES LINKER_LANGUAGE CUDA)
else()
    set_target_properties(meshfiles PROPERTIES LINKER_LANGUAGE CXX)
endif()
