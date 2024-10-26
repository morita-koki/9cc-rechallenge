#!/bin/bash

gcc -xc -c -o tmp_func.o - <<EOF
#include <stdio.h>
#include <stdlib.h>

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

void alloc4(int **p, int a, int b, int c, int d) {
  *p = malloc(4 * sizeof(int));
  (*p)[0] = a;
  (*p)[1] = b;
  (*p)[2] = c;
  (*p)[3] = d;
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





assert 3 "
int main() {
  char x[3];
  x[0] = -1;
  x[1] = 2;
  int y;
  y = 4;
  return x[0] + y;
}
"

assert 10 "
int main() {
  char x;
  x = 10;
  return x;
}
"

assert 10 "
int main() {
  char x[4];
  x[0] = 1; x[1] = 2; x[2] = 3; x[3] = 4;
  return *(x + 3) + 6;
}
"

assert 10 "
int a[4];
int main() {
  a[0] = 1; a[1] = 3; a[2] = 5; a[3] = 10;
  return a[3];
}
"

assert 10 "
int a[4];
int main() {
  *a = 1; *(a + 1) = 3; *(a + 2) = 5; *(a + 3) = 10;
  return a[3];
}
"

assert 10 "
int a[4];
int main() {
  *a = 1; *(a + 1) = 3; *(a + 2) = 5; *(a + 3) = 10;
  return *(a + 3);
}
"

assert 10 "
int a[4];
int main() {
  a[0] = 1; a[1] = 3; a[2] = 5; a[3] = 10;
  return *(a + 3);
}
"

assert 10 "
int a[4];
int main() {
  init_a();
  return *(a + 3);
}

int init_a() {
  a[0] = 1; a[1] = 3; a[2] = 5; a[3] = 10;
}
"


assert 10 "
int main() {
  int a[4];
  *(a) = 1;
  *(a + 1) = 3;
  *(a + 2) = 5;
  *(a + 3) = 10;
  return *(a + 3);
}
"

assert 10 "
int main() {
  int a[4];
  a[0] = 1;
  a[1] = 3;
  a[2] = 5;
  a[3] = 10;
  return a[3];
}
"

assert 10 "
int main() {
  int a[4];
  a[0] = 1;
  a[1] = 3;
  a[2] = 5;
  a[3] = 10;
  return *(a + 3);
}
"

assert 10 "
int main() {
  int a[4];
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  a[3] = 4;
  int b = sum(a, 4);
  return b;
}

int sum(int *a, int size) {
  int sum = 0;
  int i;
  for (i = 0; i < size; i = i + 1) {
    sum = sum + *(a + i);
  }
  return sum;
}
"

# exit 0


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


assert 10 "
int main() {
  int a;
  int *b;
  a = 10;
  b = &a;
  return *b;
}
"

assert 10 "
int main() {
  int a = 5;
  int b = 10;
  int *c = &a;
  int *d = &b;
  return *d;
}
"

assert 10 "
int main() {
  int a = 10;
  int *b = &a;
  int **c = &b;
  return **c;
}
"


assert 10 "
int main() {
  int* p;
  alloc4(&p, 1, 3, 5, 10);
  int* q; 
  q = p + 3;
  return *q;
}
"

assert 5 "
int main() {
  int* p;
  alloc4(&p, 1, 3, 5, 10);
  int* q = p + 2; 
  return *q;
}
"

assert 3 "
int main() {
  int* p;
  alloc4(&p, 1, 3, 5, 10);
  return *(p + 1);
}
"

# general address operation
assert 10 "
int main() {
  int* p;
  alloc4(&p, 1, 3, 5, 10);
  int **q = &p;
  return *(*q + 3);
}
"


# sizeof
assert 4 "
int main() {
  return sizeof(1);
}
"

assert 8 "
int main() {
  int a = 1;
  return sizeof(&a);
}
"



assert 8 "
int main() {
  int a = 1;
  return sizeof(&a + 1);
}
"


assert 40 "
int main() {
  int a[10];
  return sizeof(a);  
}
"


assert 10 "
int main() {
  int a[4];
  *(a) = 1;
  *(a + 1) = 3;
  *(a + 2) = 5;
  *(a + 3) = 10;
  return *(a + 3);
}
"



assert 3 "
int main() {
  int a[2];
  *a = 1;
  *(a + 1) = 2;
  int *p;
  p = a;
  return *p + *(p + 1);
}
"

assert 3 "
int main() {
  int a[2];
  *a = 1;
  a[1] = 2;
  int *p;
  p = a;
  return a[0] + *(p + 1);
}
"

echo OK