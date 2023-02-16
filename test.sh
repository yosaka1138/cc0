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

# add pointer
# pointer summation return next pointer
assert 4 "int main(){
    int *p;
    alloc4(&p, 1, 2, 4, 8);
    int *q;
    q = p + 2;
    return *q;
}
"

assert 8 "int main(){
    int *p;
    alloc4(&p, 1, 2 ,4, 8);
    int *q;
    q = p + 3;
    return *q;
}"
assert 2 "int main(){
    int *p;
    alloc4(&p, 1, 2 ,4, 8);
    int *q;
    q = p + 3;
    q = q - 2;
    return *q;
}"

# int
assert 1 "int main() {
    int x;
    x = 1;
    return x;
}"

assert 5 "int main(){
    int x;
    x = 3;
    return func(x, 2);
}
int func(int x, int y) {
  return x + y;
}
"

# test single integer
assert 0 'int main() {return 0;}'
assert 3 "int main() {return func(1,2);}
int func(int a, int b) {return a + b;}"
assert 4 "int main() {return func(1,2,3);}
int func(int a, int b, int c) {return a + c ;}"
assert 55 "
int plus_recur(int n) {
  if (n == 0){
    return 0;
  } else {
  return n + plus_recur(n - 1);
  }
}
int main() {
  return plus_recur(10);
}
"
assert 55 "
int plus_recur(int n) {
  if (n == 0){
    return 0;
  } else {
  return n + plus_recur(n - 1);
  }
}
int main() {
  int a;
  a = 10;
  return plus_recur(a);
}
"
assert 89 "
int main(){
  return fib(10);
}

int fib(int n){
  if (n < 2) {
    return 1;
  }
  return fib(n-1) + fib(n-2);
}
"

assert 42 'int main() return 42;'
assert 255 'int main() return 255;'
# assert 53 123
# test multi integer with plus and minus operator
assert 21 'int main() return 5+20-4;'
assert 30 'int main() return 5 + 20 - 4 + 9;'
assert 15 'int main() return 20 - 30 + 25;'
assert 47 'int main() return 5 + 6 * 7;'
assert 15 'int main() return 5 * (9 -6);'
assert 4 'int main() return (3 + 5) / 2;'
assert 8 'int main() return (3 * 5 + 1) /2;'
assert 10 'int main() return - -10;'
assert 10 'int main() return - - +10;'

assert 0 'int main() return 0==1;'
assert 1 'int main() return 42 == 42;'
assert 1 'int main() return 0!=1;'
assert 0 'int main() return 42 != 42;'

assert 1 'int main() return 0<1;'
assert 0 'int main() return 1 < 1;'
assert 1 'int main() return 0!=1;'
assert 0 'int main() return 1 != 1;'
assert 0 'int main() return 2 < 1;'
assert 1 'int main() return 1<=1;'
assert 0 'int main() return 2 <= 1;'

assert 1 'int main() return 1>0;'
assert 0 'int main() return 1 > 1;'
assert 0 'int main() return 1>2;'
assert 1 'int main() return 1 >= 0;'
assert 1 'int main() return 1>=1;'
assert 0 'int main() return 1 >= 2;'

# 1文字変数
assert 5 'int main(){
  int a;
  a =3 + 2;
  return a;
}'
assert 23 'int main() {int a; int b; a = 3; b = 20; return a + b;}'
assert 14 "int main(){ int a; int b; a = 3; b = 5 * 6 - 8; return a + b / 2;}"

# 複数文字
assert 6 "int main()  {int foo; int bar; foo=1; bar = 2 + 3; return foo + bar;}"

# return文
assert 14 "int main() {int a; int b; a = 3; b = 5 * 6 - 8; return a + b / 2;}"
assert 3 "int main() {int a; a = 3; if (a == 3) return a;
return 5;}"

assert 5 "int main() {int a; a = 3; if (a == 5) return a;
return 5;}"

assert 3 "int main(){ if (3 == 3) return 3;
else return 5;}"

assert 5 "int main() {if (3 != 3) return 3;
else return 5;}"

assert 11 "int main() {int a; a = 0;
while (a <= 10) a = a + 1;
return a;}"

assert 55 "int main() {int a;a = 0; int i;
for (i = 1; i <= 10; i = i + 1) a = a + i;
return a;}"

assert 10 "int main(){int a; a = 0;
for (; a < 10; ) a = a + 1;
return a;}"

assert 1 "int main(){int a; a = 1;
for (;;) return a;}"

# multi control syntax
assert 3 "int main(){int a; a = 3;
if (a == 2) return a;
if (a != 3) return a;
if (a == 3) return a;
return 1;}"

# block statement
assert 10 "int main(){int a; a = 0;
for (;;) {
    a = a + 1;
    if (a == 5){
        return 10;
    }
}
return 2;}"

assert 100 "int main(){int a; int i; int j; a = 0;
for (i = 0; i < 10; i = i + 1){
    for (j = 0; j < 10; j = j + 1){
        a = a + 1;
    }
}
return a;}"

assert 0 "int main(){return foo();}"
assert 7 "int main(){return bar(3, 4);}"
assert 6 "int main() {return bar2(1,2,3);}"

# ポインタ *, &
assert 3 "int main() {int x; int y; x = 3; y = &x; return *y;}"

assert 3 "int main(){
int x;
int y;
int z;
x = 3;
y = 5;
z = &y + 8;
return *z;
}"

assert 3 "int main(){
    int x;
    int *y;
    y = &x;
    *y = 3;
    return x;
}"
# エラーになる構文
# assert 15 '20 + asumiakana -5'
echo OK!
