#!/bin/bash

gcc -xc -c -o tmp_func.o - <<EOF
int foo() {
  return 5;
}

int add(int x, int y) {
  return x + y;
}

int sub(int x, int y) {
  return x - y;
}

int add6(int a, int b, int c, int d, int e, int f) {
  return a + b + c + d + e + f;
}
EOF


assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s tmp_func.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert  0 "int main(){return 0;}"
assert 41 "int main(){return 12 + 34 - 5;}"
assert  6 "int main(){return 1 + 2 + 3;}"
assert 47 "int main(){return 5+6*7;}"
assert 15 "int main(){return 5*(9-6);}"
assert  4 "int main(){return (3+5)/2;}"
assert 10 "int main(){return -10+20;}"
assert 10 "int main(){return - -10;}"
assert 10 "int main(){return - - +10;}"

assert 0 'int main(){return 0==1;}'
assert 1 'int main(){return 42==42;}'
assert 1 'int main(){return 0!=1;}'
assert 0 'int main(){return 42!=42;}'
assert 1 'int main(){return 0<1;}'
assert 0 'int main(){return 1<1;}'
assert 0 'int main(){return 2<1;}'
assert 1 'int main(){return 0<=1;}'
assert 1 'int main(){return 1<=1;}'
assert 0 'int main(){return 2<=1;}'
assert 1 'int main(){return 1>0;}'
assert 0 'int main(){return 1>1;}'
assert 0 'int main(){return 1>2;}'
assert 1 'int main(){return 1>=0;}'
assert 1 'int main(){return 1>=1;}'
assert 0 'int main(){return 1>=2;}'

assert 3 '
int main(){
  int a=3;
  return a;
}'

assert 3 '
int main() {
  int a; 
  a = 3;
  return a;
}'

assert 7 '
int main(){
  int a=3; 
  int z=5; 
  int c=7;
  return c;
}'

assert 8 '
int main(){
  int a=3;
  int z=5; 
  a+z;
  return a+z;
}'

assert 8 '
int main(){
  int a=3; 
  int z=5; 
  int b=a+z;
  return b;
}'

assert 5 "
int main(){
  int foo = 5;
  return foo;
}"

assert 8 "
int main(){
  int bar = 8; 
  return bar;
}
"

assert 10 "
int main(){
  int foo = 4;
  int bar = 6;
  return foo + bar;
}
"

assert 4 "int main(){ return 4;}"
assert 5 "int main(){int foo = 5; return foo;}"
assert 8 "
int main(){
  int bar = 8; 
  return bar;
}
"

assert 10 "
int main(){
  int bar = 8; 
  return bar + 2;
}
"

assert 10 "
int main(){
  int foo = 4;
  int bar = 6;
  return foo + bar;
}
"

# if else statement
assert 10 "
int main(){
  int foo = 4;
  int bar = 6;
  if (foo == 4) {
    return foo + bar;
  }
  return 0;
}
"

assert 10 "
int main(){
  int foo = 4;
  int bar = 6;
  if (1 == 5) return 3;
  return foo + bar;
}
"

assert 10 "
int main(){
  int foo = 4;
  int bar = 6;
  if (foo == 4) return 10;
  else return 0;
}
"

assert 10 "
int main(){
  int foo = 4;
  int bar = 6;
  if (foo == 5) return 0;
  else return foo + bar;
}
"


# while statement
assert 10 "
int main(){
  int foo = 4;
  int bar = 6;
  while (foo < 10) { 
    foo = foo + 1;
  }
  return foo;
}
"

assert 10 "
int main(){
  int foo = 4;
  int bar = 6;
  while (foo < 10) {
    return 10;
  }
  return foo;
}
"

# for statement
assert 10 "
int main(){
  int foo = 0;
  int bar = 6;
  int i;
  for (i = 0; i < 10; i = i + 1) {
    foo = foo + 1;
  }
  return foo;
}
"

assert 10 "
int main(){
  for (;;) return 10;
  return 5;
}
"

# block statement
assert 10 "
int main(){
  int foo = 4;
  int bar = 6;
  if (foo == 4) return 10;
  else return 3;
}
"

assert 10 "
int main(){
  int foo = 4;
  int bar = 6;
  if (foo == 5) return 0;
  else return foo + bar;
}
"

assert 10 "
int main(){
  {
    int foo = 4;
    int bar = 6;
  }
  return 10;
}
"


assert  5 "int main(){ return foo(); }"
assert 10 "int main(){ return foo() + 5; }"

assert 10 "int main(){ return add(4, 6); }"
assert 10 "int main(){ return add(4, add(2, 4)); }"
assert 10 "int main(){ return add(add(2, 4), 4); }"

assert 10 "int main(){ return sub(20, 10); }"
assert 10 "int main(){ return sub(sub(20, 5), 5); }"

assert 21 "int main(){ return add6(1, 2, 3, 4, 5, 6); }"

assert 10 "
int main(){
  int a = 3;
  int b = 7;
  return huga();
}
int huga() {
  return 10;
}
"

assert 10 "
int main(){
  int foo = 3;
  int bar = 7;
  return add2(foo, bar);
}
int add2(int x, int y) {
  return x + y;
}
"

assert 10 "
int main(){
  int foo = 20;
  int bar = 10;
  return sub2(foo, bar);
}
int sub2(int x, int y) {
  return (x - y);
}
"

assert 10 "
int main(){
  return func(4);
}
int func(int x) {
  if (x == 1) return 1;
  return func(x-1) + x;
}
"

assert 10 "
int main(){
  int huga = 4;
  return ababa(huga);
}
int ababa(int x) {
  int cicic = 2;
  return x + mkmk(cicic);
}
int mkmk(int x) {
  return x * 4 - 2;
}
"

assert 10 "
int main() {
  return bar(1,2,3,1,1,2);
}
int bar(int a, int b, int c, int d, int e, int f) {
  return a + b + c + d + e + f;
}
"

# assert 10 "
# int main() {
#   a = 5;
#   b = 10;
#   c = &a;
#   d = &b;
#   return *&b;
# }
# "

# assert 10 "int main() { a=10; return *&a; }"
# assert 10 "int main() { a=10; b=&a; c=&b; return **c; }"


# error check
# assert 6 "1 + 2 ++ 3"
# assert 8 "1 + 2 + 3 3"

echo OK