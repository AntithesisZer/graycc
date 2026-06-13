add_library(AST INTERFACE)
target_include_directories(AST
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)