#pragma once

#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include <AST.hpp>
#include <Token.hpp>

struct LVar {
    std::string_view name;
    int offset;
};

class Parser {
public:
    explicit Parser(std::span<const Token> tokens, ASTContext& ctx, std::string_view expr_str)
        : m_tokens(tokens), m_ctx(ctx), m_expr_str(expr_str) {}

    auto parser() -> std::vector<Node*> {
        std::vector<Node*> stmts;
        while (current().kind != TokenKind::TK_EOF) {
            stmts.push_back(stmt());
        }
        return stmts;
    }

    auto get_stack_size() const -> int {
        return (static_cast<int>(m_locals.size()) * 8 + 15) & ~15;
    }

private:
    std::span<const Token> m_tokens;
    ASTContext& m_ctx;
    std::string_view m_expr_str;
    std::size_t m_cursor{ 0 };
    std::vector<LVar> m_locals;

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

    auto find_lvar(std::string_view name) -> LVar* {
        for (auto& var : m_locals) {
            if (var.name == name) {
                return &var;
            }
        }
        return nullptr;
    }

    auto stmt() -> Node* {
        if (match("return")) {
            Node* node = m_ctx.make_binary(NodeKind::ND_RETURN, expr(), nullptr);
            expect(";");
            return node;
        }
        Node* node = expr();
        expect(";");
        return node;
    }

    auto expr() -> Node* {
        return assign();
    }

    auto assign() -> Node* {
        Node* node = equality();
        if (match("=")) {
            node = m_ctx.make_binary(NodeKind::ND_ASSIGN, node, assign());
        }
        return node;
    }

    auto equality() -> Node* {
        Node* node = relational();
        while (auto op = match("==", "!=")) {
            if (*op == "==") {
                node = m_ctx.make_binary(NodeKind::ND_EQ, node, relational());
            } else {
                node = m_ctx.make_binary(NodeKind::ND_NE, node, relational());
            }
        }
        return node;
    }

    auto relational() -> Node* {
        Node* node = add();
        while (auto op = match("<", "<=", ">", ">=")) {
            if (*op == "<") {
                node = m_ctx.make_binary(NodeKind::ND_LT, node, add());
            } else if (*op == "<=") {
                node = m_ctx.make_binary(NodeKind::ND_LE, node, add());
            } else if (*op == ">") {
                node = m_ctx.make_binary(NodeKind::ND_LT, add(), node);
            } else {
                node = m_ctx.make_binary(NodeKind::ND_LE, add(), node);
            }
        }
        return node;
    }

    auto add() -> Node* {
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

        if (current().kind == TokenKind::TK_IDENT) {
            std::string_view name = current().loc;
            advance();

            LVar* var = find_lvar(name);
            if (!var) {
                int offset = (static_cast<int>(m_locals.size()) + 1) * 8;
                m_locals.push_back(LVar{
                    .name = name,
                    .offset = offset,
                });
                var = &m_locals.back();
            }
            return m_ctx.make_lvar(var->offset);
        }
        return m_ctx.make_num(expect_number());
    }
};