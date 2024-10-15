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

assert  0 "main(){return 0;}"
assert 41 "main(){return 12 + 34 - 5;}"
assert  6 "main(){return 1 + 2 + 3;}"
assert 47 "main(){return 5+6*7;}"
assert 15 "main(){return 5*(9-6);}"
assert  4 "main(){return (3+5)/2;}"
assert 10 "main(){return -10+20;}"
assert 10 "main(){return - -10;}"
assert 10 "main(){return - - +10;}"

assert 0 'main(){return 0==1;}'
assert 1 'main(){return 42==42;}'
assert 1 'main(){return 0!=1;}'
assert 0 'main(){return 42!=42;}'
assert 1 'main(){return 0<1;}'
assert 0 'main(){return 1<1;}'
assert 0 'main(){return 2<1;}'
assert 1 'main(){return 0<=1;}'
assert 1 'main(){return 1<=1;}'
assert 0 'main(){return 2<=1;}'
assert 1 'main(){return 1>0;}'
assert 0 'main(){return 1>1;}'
assert 0 'main(){return 1>2;}'
assert 1 'main(){return 1>=0;}'
assert 1 'main(){return 1>=1;}'
assert 0 'main(){return 1>=2;}'

assert 3 '
main(){
a=3;
return a;
}'

assert 3 '
main(){
a=3; 
a;
return a;
}'

assert 7 '
main(){
a=3; z=5; c=7;
return c;
}'

assert 8 '
main(){
a=3; z=5; a+z;
return a+z;
}'

assert 8 '
main(){
a=3; z=5; b=a+z;
return b;
}'

assert 5 "
main(){
foo = 5;
return foo;
}"

assert 8 "
main(){
bar = 8; 
return bar;
}
"

assert 10 "
main(){
foo = 4;
bar = 6;
return foo + bar;
}
"

assert 4 "main(){return 4;}"
assert 5 "main(){foo = 5; return foo;}"
assert 8 "
main(){
bar = 8; 
return bar;
}
"

assert 10 "
main(){
bar = 8; 
return bar + 2;
}
"

assert 10 "
main(){
foo = 4;
bar = 6;
return foo + bar;
}
"

# if else statement
assert 10 "
main(){
foo = 4;
bar = 6;
if (foo == 4) return foo + bar;
return 0;
}
"

assert 10 "
main(){
foo = 4;
bar = 6;
if (1 == 5) return 3;
return foo + bar;
}
"

assert 10 "
main(){
foo = 4;
bar = 6;
if (foo == 4) return 10;
else return 0;
}
"

assert 10 "
main(){
foo = 4;
bar = 6;
if (foo == 5) return 0;
else return foo + bar;
}
"


# while statement
assert 10 "
main(){
foo = 4;
bar = 6;
while (foo < 10) foo = foo + 1;
return foo;
}
"

assert 10 "
main(){
foo = 4;
bar = 6;
while (foo < 10) return 10;
return foo;
}
"

# for statement
assert 10 "
main(){
foo = 0;
bar = 6;
for (i = 0; i < 10; i = i + 1) foo = foo + 1;
return foo;
}
"

assert 10 "
main(){
for (;;) return 10;
return 5;
}
"

# block statement
assert 10 "
main(){

  foo = 4;
  bar = 6;
  if (foo == 4) return 10;
  else return 3;

}
"

assert 10 "
main(){

  foo = 4;
  bar = 6;
  if (foo == 5) return 0;
  else return foo + bar;

}
"

assert 10 "
main(){
{
  foo = 4;
  bar = 6;
}
return 10;
}
"


assert 5 "main(){ return foo(); }"
assert 10 "main(){ return foo() + 5; }"

assert 10 "main(){ return add(4, 6); }"
assert 10 "main(){ return add(4, add(2, 4)); }"
assert 10 "main(){ return add(add(2, 4), 4); }"

assert 10 "main(){ return sub(20, 10); }"
assert 10 "main(){ return sub(sub(20, 5), 5); }"

assert 21 "main(){ return add6(1, 2, 3, 4, 5, 6); }"

assert 10 "
main(){
  a = 3;
  b = 7;
  return huga();
}
huga() {
  return 10;
}
"

assert 10 "
main(){
  foo = 3;
  bar = 7;
  return add2(foo, bar);
}
add2(x, y) {
  return x + y;
}
"

assert 10 "
main(){
  foo = 20;
  bar = 10;
  return sub2(foo, bar);
}
sub2(x, y) {
  return (x - y);
}
"

assert 10 "
main(){
  return func(4);
}
func(x) {
  if (x == 1) return 1;
  return func(x-1) + x;
}
"

assert 10 "
main(){
  huga = 4;
  return ababa(huga);
}
ababa(x) {
  cicic = 2;
  return x + mkmk(cicic);
}
mkmk(x) {
  return x * 4 - 2;
}
"

assert 10 "
main() {
  return bar(1,2,3,1,1,2);
}
bar(a,b,c,d,e,f) {
  return a + b + c + d + e + f;
}
"

assert 10 "
main() {
  a = 5;
  b = 10;
  c = &a;
  d = &b;
  return *&b;
}
"

assert 10 "main() { a=10; return *&a; }"
assert 10 "main() { a=10; b=&a; c=&b; return **c; }"


# error check
# assert 6 "1 + 2 ++ 3"
# assert 8 "1 + 2 + 3 3"

echo OK