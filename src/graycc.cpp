#include <print>
#include <span>

int main(int argc, char* argv[]) {
    std::span<const char* const> args = std::span(argv, argc);
    if (argc != 2) {
        std::println(stderr, "Usage: {} <num>", args[0]);
        return 1;
    }

    char* p = argv[1];

    std::println(".intel_syntax noprefix");
    std::println(".globl main");
    std::println();
    std::println("main:");
    std::println("    mov rax, {}", std::strtol(p, &p, 10));

    while (*p) {
        if (*p == '+') {
            ++p;
            std::println("    add rax, {}", std::strtol(p, &p, 10));
            continue;
        }
        if (*p == '-') {
            ++p;
            std::println("    sub rax, {}", std::strtol(p, &p, 10));
            continue;
        }
        std::println("Invalid char: '{}'", *p);
        return 1;
    }

    std::println("    ret");
}