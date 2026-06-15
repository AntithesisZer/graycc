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

    Node* root = parser.parser();

    std::println(".intel_syntax noprefix");
    std::println(".globl main");
    std::println();
    std::println("main:");

    codegen(root);

    std::println("    pop rax");
    std::println("    ret");
}