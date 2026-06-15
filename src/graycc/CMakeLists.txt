add_executable(graycc)
target_compile_features(graycc PRIVATE cxx_std_23)
target_sources(graycc
    PRIVATE
        graycc.cpp
)
target_link_libraries(graycc
    PRIVATE
        AST
        Parser
        Token
)