#include <print>
#include <span>
#include <string_view>
#include <vector>

#include <Token.hpp>

int main(int argc, char* argv[]) {
    std::span<const char* const> args = std::span(argv, argc);
    if (argc != 2) {
        std::println(stderr, "Usage: {} <expression>", args[0]);
        std::exit(1);
    }

    std::string_view expression = args[1];
    std::vector<Token> tokens = Tokenize(args[1]);

    std::println(".intel_syntax noprefix");
    std::println(".globl main");
    std::println();
    std::println("main:");

    if (tokens.empty() || tokens[0].kind != TokenKind::TK_NUM) {
        // std::println(stderr, "Error: Expression must start with a number");
        // std::exit(1);
        error_at(tokens[0].loc, expression, "Expression must start with a number");
    }
    std::println("    mov rax, {}", tokens[0].val);

    for (std::size_t i = 1; i < tokens.size(); /**/) {
        const Token& tok = tokens[i];

        if (tok.kind == TokenKind::TK_EOF) {
            break;
        }

        if (tok.kind != TokenKind::TK_RESERVED) {
            // std::println(stderr, "Error: Expected operator '{}'", tok.loc);
            // std::exit(1);
            error_at(tokens[i].loc, expression, "Expected operator");
        }

        if (i + 1 > tokens.size() || tokens[i + 1].kind != TokenKind::TK_NUM) {
            // std::println(stderr, "Error: Expected number after operator '{}'", tokens[i + 1].loc);
            // std::exit(1);
            error_at(tokens[i + 1].loc, expression, "Expected number after operator");
        }

        if (const Token& next_tok = tokens[i + 1]; tok.loc == "+") {
            std::println("    add rax, {}", next_tok.val);
        } else if (tok.loc == "-") {
            std::println("    sub rax, {}", next_tok.val);
        }

        i += 2;
    }

    std::println("    ret");
}