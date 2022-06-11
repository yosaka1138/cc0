#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./cc0 "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

# test single integer
assert 0 0
assert 42 42
assert 255 255
# assert 53 123
# test multi integer with plus and minus operator
assert 21 '5+20-4'
assert 30 '5 + 20 - 4 + 9'
assert 15 '20 - 30 + 25'
assert 47 '5 + 6 * 7'
assert 15 '5 * (9 -6)'
assert 4 '(3 + 5) / 2'
assert 8 '(3 * 5 + 1) /2'
// エラーになる構文
assert 15 '20 + asumiakana -5'
echo OK!
