# Triangle mesh generator
cmake_minimum_required(VERSION 3.10)

# Set Triangle source files
set(TRIANGLE_SOURCES
    triangle.c
)

# Create Triangle executable
add_executable(triangle ${TRIANGLE_SOURCES})

# Set compile definitions for Triangle based on platform
if(WIN32)
    target_compile_definitions(triangle PRIVATE
        NO_TIMER        # Remove Unix timer for Windows compatibility
        CPU86           # Enable x86 optimizations for Windows
    )
else()
    target_compile_definitions(triangle PRIVATE
        NO_TIMER        # Remove Unix timer (cross-platform compatibility)
        LINUX           # Enable Linux-specific optimizations
    )
endif()

# Link math library (only on Unix systems)
if(NOT WIN32)
    target_link_libraries(triangle m)
endif()

# Set output directory to build/bin so it's easily accessible
set_target_properties(triangle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Optional: Create triangle library for potential future integration
add_library(triangle_lib STATIC ${TRIANGLE_SOURCES})
if(WIN32)
    target_compile_definitions(triangle_lib PRIVATE
        TRILIBRARY      # Compile as library
        NO_TIMER
        CPU86
    )
else()
    target_compile_definitions(triangle_lib PRIVATE
        TRILIBRARY      # Compile as library
        NO_TIMER
        LINUX
    )
endif()

# Set library output directory
set_target_properties(triangle_lib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
) 