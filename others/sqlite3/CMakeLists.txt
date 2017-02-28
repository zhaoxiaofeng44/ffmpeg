
set(SQLITE3_SRC
    cplusplus/shell.c 
    cplusplus/sqlite3.c 
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/cplusplus
)

add_library(sqlite3 STATIC
  ${SQLITE3_SRC}
)

set(CMAKE_C_FLAGS_DEBUG "-g -Wall -ldl")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -ldl")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set_target_properties(sqlite3
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
