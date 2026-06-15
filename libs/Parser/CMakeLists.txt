add_library(Parser INTERFACE)
target_include_directories(Parser
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)