#pragma once

#include <cstdint>
#include <memory_resource>

enum class NodeKind : std::uint64_t {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
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