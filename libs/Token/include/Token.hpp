#pragma once

#include <charconv>
#include <cstdint>
#include <print>
#include <string_view>
#include <vector>

enum class TokenKind : std::uint64_t {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
};

struct alignas(32) Token {
    TokenKind kind;
    std::string_view loc;
    std::uint64_t val{ 0 };
};

[[nodiscard]]
inline auto Tokenize(std::string_view input) -> std::vector<Token> {
    std::vector<Token> tokens;
    tokens.reserve(input.size() / 6 + 1);

    while (!input.empty()) {
        if (std::isspace(input.front())) {
            input.remove_prefix(1);
            continue;
        }

        if (input.starts_with("==") || input.starts_with("!=") ||
            input.starts_with("<=") || input.starts_with(">=")) {
            tokens.push_back(Token{
                .kind = TokenKind::TK_RESERVED,
                .loc = input.substr(0, 2),
            });
            input.remove_prefix(2);
            continue;
        }

        if (auto ch = input.front();
            ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
            ch == '(' || ch == ')' || ch == '<' || ch == '>' ||
            ch == '=' || ch == ';') {
            tokens.push_back(Token{
                .kind = TokenKind::TK_RESERVED,
                .loc = input.substr(0, 1),
            });
            input.remove_prefix(1);
            continue;
        }

        if (input.starts_with("return") && (input.size() == 6 || (!std::isalnum(input[6]) && input[6] != '_'))) {
            tokens.push_back(Token{
                .kind = TokenKind::TK_RESERVED,
                .loc = input.substr(0, 6),
            });
            input.remove_prefix(6);
            continue;
        }

        if (auto ch = input.front(); std::isalpha(ch) || ch == '_') {
            std::size_t len{ 1 };
            while (len < input.size() && (std::isalnum(input[len]) || input[len] == '_')) {
                ++len;
            }
            tokens.push_back(Token{
                .kind = TokenKind::TK_IDENT,
                .loc = input.substr(0, len),
            });
            input.remove_prefix(len);
            continue;
        }

        if (std::isdigit(input.front())) {
            std::uint64_t val{ 0 };
            auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), val);
            if (ec != std::errc{}) {
                std::println(stderr, "Error: Invalid number format");
                std::exit(1);
            }
            tokens.push_back(Token{
                .kind = TokenKind::TK_NUM,
                .loc = input.substr(0, static_cast<std::size_t>(ptr - input.data())),
                .val = val,
            });
            input.remove_prefix(static_cast<std::size_t>(ptr - input.data()));
            continue;
        }

        std::println(stderr, "Error Invalid token: {}", input.front());
        std::exit(1);
    }

    tokens.push_back(Token{
        .kind = TokenKind::TK_EOF,
        .loc = "",
    });
    return tokens;
}

inline auto error_at(std::string_view loc, std::string_view expr, std::string_view info) -> void {
    std::println(stderr, "{}", expr);
    std::println(stderr, "{:>{}}{:^^{}}", "", static_cast<std::size_t>(loc.data() - expr.data()), "", loc.size());
    std::println(stderr, "{:>{}}{}", "", static_cast<std::size_t>(loc.data() - expr.data()), info);
    std::exit(1);
}