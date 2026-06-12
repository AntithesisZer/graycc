add_library(Token INTERFACE)
target_include_directories(Token
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)