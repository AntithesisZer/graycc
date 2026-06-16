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

assert 17 "11 + 6"
assert 2 "21 - 19"
assert 21 "5 + 20 - 4"
assert 33 "42 - 16 + 7"
assert 13 "3 + 5 * 2"
assert 4 "24 / (8 - 2)"
assert 13 "-8 + 21"
assert 41 "27 - -6 + +(4 * 2)"
assert 1 "5 * 4 == 20"
assert 0 "3 + 2 <= -1"
assert 1 "(10 > 5) + (2 != 3) == 2"

echo OK