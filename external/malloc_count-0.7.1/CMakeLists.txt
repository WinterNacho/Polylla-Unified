file(GLOB malloc_count_SRC
     "*.h"
     "*.c"
)

add_library(malloccountfiles STATIC ${malloc_count_SRC})

# Set language to C for this library
set_target_properties(malloccountfiles PROPERTIES 
    LINKER_LANGUAGE C
    C_STANDARD 99
)