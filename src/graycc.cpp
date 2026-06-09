#include <print>
#include <span>

int main(int argc, char* argv[]) {
    std::span<const char* const> args = std::span(argv, argc);
    if (argc != 2) {
        std::println(stderr, "Usage: {} <num>", args[0]);
        return 1;
    }

    std::println(".intel_syntax noprefix");
    std::println(".globl main");
    std::println();
    std::println("main:");
    std::println("    mov rax, {}", args[1]);
    std::println("    ret");
}