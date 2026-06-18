#pragma once

#include <cassert>
#include <cstdint>
#include <memory_resource>
#include <print>
#include <utility>

enum class NodeKind : std::uint64_t {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_ASSIGN,
    ND_LVAR,
    ND_NUM,
};

struct alignas(32) Node {
    NodeKind kind;
    alignas(16) union {
        struct {
            Node* lhs;
            Node* rhs;
        };
        std::uint64_t val;
    };
};

class ASTContext {
public:
    ASTContext() : m_pool(std::pmr::new_delete_resource()), m_alloc(&m_pool) {}
    ASTContext(const ASTContext&) = delete;
    auto operator=(const ASTContext&) -> ASTContext& = delete;
    ~ASTContext() = default;

    auto make_num(std::uint64_t val) -> Node* {
        void* mem = m_alloc.allocate(1);
        return ::new (mem) Node{
            .kind = NodeKind::ND_NUM,
            .val = val,
        };
    }
    auto make_lvar(std::uint64_t offset) -> Node* {
        void* mem = m_alloc.allocate(1);
        return ::new (mem) Node{
            .kind = NodeKind::ND_LVAR,
            .val = offset,
        };
    }
    auto make_binary(NodeKind kind, Node* lhs, Node* rhs) -> Node* {
        void* mem = m_alloc.allocate(1);
        return ::new (mem) Node{
            .kind = kind,
            .lhs = lhs,
            .rhs = rhs,
        };
    }

private:
    std::pmr::monotonic_buffer_resource m_pool;
    std::pmr::polymorphic_allocator<Node> m_alloc;
};

inline auto codegen(const Node* node) -> void;

inline auto codegen_lval(const Node* node) -> void {
    if (node->kind != NodeKind::ND_LVAR) {
        std::println(stderr, "Error: Left-hand side of assignment is not an lvalue");
        std::exit(1);
    }
    std::println("    mov rax, rbp");
    std::println("    sub rax, {}", node->val);
    std::println("    push rax");
}

inline auto emit_basic_binary(const Node* node, std::string_view asm_ops) -> void {
    codegen(node->lhs);
    codegen(node->rhs);
    std::println("    pop rdi");
    std::println("    pop rax");
    std::println("    {} rax, rdi", asm_ops);
    std::println("    push rax");
}

inline auto emit_compare_binary(const Node* node, std::string_view set_op) -> void {
    codegen(node->lhs);
    codegen(node->rhs);
    std::println("    pop rdi");
    std::println("    pop rax");
    std::println("    cmp rax, rdi");
    std::println("    {} al", set_op);
    std::println("    movzx rax, al");
    std::println("    push rax");
}

inline auto codegen(const Node* node) -> void {
    assert(node != nullptr && "Internal Error: codegen received a nullptr node");

    switch (node->kind) {
    case NodeKind::ND_NUM:
        std::println("    push {}", node->val);
        return;
    case NodeKind::ND_LVAR:
        codegen_lval(node);
        std::println("    pop rax");
        std::println("    mov rax, [rax]");
        return;
    case NodeKind::ND_ASSIGN:
        codegen_lval(node->lhs);
        codegen(node->rhs);
        std::println("    pop rdi");
        std::println("    pop rax");
        std::println("    mov [rax], rdi");
        std::println("    push rdi");
        return;
    case NodeKind::ND_ADD:
        emit_basic_binary(node, "add");
        break;
    case NodeKind::ND_SUB:
        emit_basic_binary(node, "sub");
        break;
    case NodeKind::ND_MUL:
        emit_basic_binary(node, "imul");
        break;
    case NodeKind::ND_DIV:
        codegen(node->lhs);
        codegen(node->rhs);
        std::println("    pop rdi");
        std::println("    pop rax");
        std::println("    cqo");
        std::println("    idiv rdi");
        std::println("    push rax");
        break;
    case NodeKind::ND_EQ:
        emit_compare_binary(node, "sete");
        break;
    case NodeKind::ND_NE:
        emit_compare_binary(node, "setne");
        break;
    case NodeKind::ND_LT:
        emit_compare_binary(node, "setl");
        break;
    case NodeKind::ND_LE:
        emit_compare_binary(node, "setle");
        break;
    default:
        std::unreachable();
    }
}