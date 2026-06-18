#include <print>
#include <span>
#include <string_view>
#include <vector>

#include <AST.hpp>
#include <Parser.hpp>
#include <Token.hpp>

int main(int argc, char* argv[]) {
    std::span<const char* const> args = std::span(argv, argc);
    if (argc != 2) {
        std::println(stderr, "Usage: {} <expression>", args[0]);
        std::exit(1);
    }

    std::string_view expression = args[1];
    std::vector<Token> tokens = Tokenize(expression);

    ASTContext ctx;
    Parser parser(tokens, ctx, expression);

    std::vector<Node*> stmts = parser.parser();
    int stack_size = parser.get_stack_size();

    std::println(".intel_syntax noprefix");
    std::println(".globl main");
    std::println();
    std::println("main:");

    std::println("    push rbp");
    std::println("    mov rbp, rsp");
    std::println("    sub rsp, {}", stack_size);

    for (const auto* stmt : stmts) {
        codegen(stmt);
        std::println("    pop rax");
    }

    std::println("    mov rsp, rbp");
    std::println("    pop rbp");
    std::println("    ret");
}