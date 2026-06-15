#pragma once

#include <optional>
#include <span>
#include <string_view>

#include <AST.hpp>
#include <Token.hpp>

class Parser {
public:
    explicit Parser(std::span<const Token> tokens, ASTContext& ctx, std::string_view expr_str)
        : m_tokens(tokens), m_ctx(ctx), m_expr_str(expr_str) {}

    auto parser() -> Node* {
        Node* root = expr();
        if (current().kind != TokenKind::TK_EOF) {
            error_at(current().loc, m_expr_str, "Extra tokens after valid expression");
        }
        return root;
    }

private:
    std::span<const Token> m_tokens;
    ASTContext& m_ctx;
    std::string_view m_expr_str;
    std::size_t m_cursor{ 0 };

    [[nodiscard]]
    auto current() const -> const Token& {
        return m_tokens[m_cursor];
    }

    auto advance() -> void {
        if (current().kind != TokenKind::TK_EOF) {
            ++m_cursor;
        }
    }

    template <typename... Args>
    auto match(Args... ops) -> std::optional<std::string_view> {
        if (current().kind == TokenKind::TK_RESERVED && ((current().loc == ops) || ...)) {
            std::string_view matched_loc = current().loc;
            advance();
            return matched_loc;
        }
        return std::nullopt;
    }

    auto expect(std::string_view op) -> void {
        if (!match(op)) {
            error_at(current().loc, m_expr_str, "Expected '" + std::string(op) + "'");
        }
    }

    auto expect_number() -> std::uint64_t {
        if (current().kind != TokenKind::TK_NUM) {
            error_at(current().loc, m_expr_str, "Expected a number");
        }
        std::uint64_t val = current().val;
        advance();
        return val;
    }

    auto expr() -> Node* {
        Node* node = mul();
        while (auto op = match("+", "-")) {
            if (*op == "+") {
                node = m_ctx.make_binary(NodeKind::ND_ADD, node, mul());
            } else {
                node = m_ctx.make_binary(NodeKind::ND_SUB, node, mul());
            }
        }
        return node;
    }

    auto mul() -> Node* {
        Node* node = unary();
        while (auto op = match("*", "/")) {
            if (*op == "*") {
                node = m_ctx.make_binary(NodeKind::ND_MUL, node, unary());
            } else {
                node = m_ctx.make_binary(NodeKind::ND_DIV, node, unary());
            }
        }
        return node;
    }

    auto unary() -> Node* {
        if (match("+")) {
            return unary();
        }
        if (match("-")) {
            return m_ctx.make_binary(NodeKind::ND_SUB, m_ctx.make_num(0), unary());
        }
        return primary();
    }

    auto primary() -> Node* {
        if (match("(")) {
            Node* node = expr();
            expect(")");
            return node;
        }
        return m_ctx.make_num(expect_number());
    }
};