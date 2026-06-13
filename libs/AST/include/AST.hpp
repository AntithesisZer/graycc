#pragma once

#include <cstdint>

enum class NodeKind : std::uint64_t {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
};

struct alignas(32) Node {
    NodeKind kind;
    std::uint64_t padding{ 0 };
    alignas(16) union {
        struct {
            Node* lhs;
            Node* rhs;
        };
        std::uint64_t val;
    };
};

inline auto alloc_binary_node(NodeKind kind, Node* lhs, Node* rhs) -> Node* {
    return new Node{
        .kind = kind,
        .lhs = lhs,
        .rhs = rhs,
    };
}

inline auto alloc_num_node(std::uint64_t val) -> Node* {
    return new Node{
        .kind = NodeKind::ND_NUM,
        .val = val,
    };
}