#!/bin/zsh
assert() {
    expected="$1"
    input="$2"

    ../bin/graycc "$input" > tmp.s
    clang -fuse-ld=lld -Wl,-z,noexecstack -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but not $actual"
        exit 1
    fi
}

assert 0 0
assert 42 42

echo OK