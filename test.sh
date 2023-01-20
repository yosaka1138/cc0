#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./cc0 "$input" > tmp.s
  cd func
  cc -c func.c
  cd ../
  cc -o tmp tmp.s func/func.o
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
assert 0 'main() {return 0;}'
assert 3 "main() {return func(1,2);}
func(a,b) {return a + b;}"
assert 4 "main() {return func(1,2,3);}
func(a,b, c) {return a + c ;}"
assert 55 "
plus_recur(n) {
  if (n == 0){
    return 0;
  } else {
  return n + plus_recur(n - 1);
  }
}
main() {
  return plus_recur(10);
}
"
assert 55 "
plus_recur(n) {
  if (n == 0){
    return 0;
  } else {
  return n + plus_recur(n - 1);
  }
}
main() {
  a = 10;
  return plus_recur(a);
}
"
assert 89 "
main(){
  return fib(10);
}
fib(n){
  if (n < 2) {
    return 1;
  }
  return fib(n-1) + fib(n-2);
}
"

# assert 42 '42;'
# assert 255 '255;'
# # assert 53 123
# # test multi integer with plus and minus operator
# assert 21 '5+20-4;'
# assert 30 '5 + 20 - 4 + 9;'
# assert 15 '20 - 30 + 25;'
# assert 47 '5 + 6 * 7;'
# assert 15 '5 * (9 -6);'
# assert 4 '(3 + 5) / 2;'
# assert 8 '(3 * 5 + 1) /2;'
# assert 10 '- -10;'
# assert 10 '- - +10;'

# assert 0 '0==1;'
# assert 1 '42 == 42;'
# assert 1 '0!=1;'
# assert 0 '42 != 42;'

# assert 1 '0<1;'
# assert 0 '1 < 1;'
# assert 1 '0!=1;'
# assert 0 '1 != 1;'
# assert 0 '2 < 1;'
# assert 1 '1<=1;'
# assert 0 '2 <= 1;'

# assert 1 '1>0;'
# assert 0 '1 > 1;'
# assert 0 '1>2;'
# assert 1 '1 >= 0;'
# assert 1 '1>=1;'
# assert 0 '1 >= 2;'

# # 1文字変数
# assert 5 'a =3 + 2;'
# assert 23 'a = 3; b = 20; a + b;'
# assert 14 "a = 3; b = 5 * 6 - 8; a + b / 2;"

# # 複数文字
# assert 6 "foo=1; bar = 2 + 3; foo + bar;"

# # return文
# assert 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;"
# assert 3 "a = 3; if (a == 3) return a;
# return 5;"

# assert 5 "a = 3; if (a == 5) return a;
# return 5;"

# assert 3 "if (3 == 3) return 3;
# else return 5;"

# assert 5 "if (3 != 3) return 3;
# else return 5;"

# assert 11 "a = 0;
# while (a <= 10) a = a + 1;
# return a;"

# assert 55 "a = 0;
# for (i = 1; i <= 10; i = i + 1) a = a + i;
# return a;"

# assert 10 "a = 0;
# for (; a < 10; ) a = a + 1;
# return a;"

# assert 1 "a = 1;
# for (;;) return a;"

# # multi control syntax
# assert 3 "a = 3;
# if (a == 2) return a;
# if (a != 3) return a;
# if (a == 3) return a;
# return 1;"

# # block statement
# assert 10 "a = 0;
# for (;;) {
#     a = a + 1;
#     if (a == 5){
#         return 10;
#     }
# }
# return 2;"

# assert 100 "a = 0;
# for (i = 0; i < 10; i = i + 1){
#     for (j = 0; j < 10; j = j + 1){
#         a = a + 1;
#     }
# }
# return a;"

# assert 0 "foo(); return 0;"
# assert 0 "bar(3, 4); return 0;"
# assert 0 "bar2(1,2,3); return 0;"

# エラーになる構文
# assert 15 '20 + asumiakana -5'
echo OK!
