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

  ./9cc $input > tmp.s
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


assert  0 "tests/00.c"
assert 41 "tests/01.c"
assert  6 "tests/02.c"
assert 47 "tests/03.c"
assert 15 "tests/04.c"
assert  4 "tests/05.c"
assert 10 "tests/06.c" 
assert 10 "tests/07.c" 
assert 10 "tests/08.c" 
assert  0 "tests/09.c" 
assert  1 "tests/10.c" 
assert  1 "tests/11.c" 
assert  0 "tests/12.c"  
assert  1 "tests/13.c"  
assert  0 "tests/14.c"  
assert  0 "tests/15.c"  
assert  1 "tests/16.c"  
assert  1 "tests/17.c"  
assert  0 "tests/18.c"  
assert  1 "tests/19.c"  
assert  0 "tests/20.c"  
assert  0 "tests/21.c"  
assert  1 "tests/22.c"  
assert  1 "tests/23.c"  
assert  0 "tests/24.c"
assert  3 "tests/25.c"
assert  3 "tests/26.c"
assert  8 "tests/27.c"
assert  8 "tests/28.c"
assert 10 "tests/29.c"
assert 10 "tests/30.c"
assert 10 "tests/31.c"
assert 10 "tests/32.c"
assert 10 "tests/33.c"
assert 10 "tests/34.c"
assert 10 "tests/35.c"
assert 10 "tests/36.c"
assert 10 "tests/37.c"
assert 10 "tests/38.c"
assert 10 "tests/39.c"
assert 10 "tests/40.c"
assert  5 "tests/41.c"
assert 10 "tests/42.c"
assert 10 "tests/43.c"
assert 10 "tests/44.c"
assert 10 "tests/45.c"
assert 10 "tests/46.c"
assert 10 "tests/47.c"
assert 21 "tests/48.c"
assert 10 "tests/49.c"
assert 10 "tests/50.c"
assert 10 "tests/51.c"
assert 10 "tests/52.c"
assert 10 "tests/53.c"
assert 10 "tests/54.c"
assert 10 "tests/55.c"
assert 10 "tests/56.c"
assert 10 "tests/57.c"
assert 10 "tests/58.c"
assert  5 "tests/59.c"
assert  3 "tests/60.c"
assert 10 "tests/61.c"
assert  4 "tests/62.c"
assert  8 "tests/63.c"
assert  8 "tests/64.c"
assert 40 "tests/65.c"
assert 10 "tests/66.c"
assert  3 "tests/67.c"
assert  3 "tests/68.c"
assert 10 "tests/69.c"
assert 10 "tests/70.c"
assert 10 "tests/71.c"
assert 10 "tests/72.c"
assert 10 "tests/73.c"
assert 10 "tests/74.c"
assert 10 "tests/75.c"
assert 10 "tests/76.c"
assert 10 "tests/77.c"
assert 10 "tests/78.c"
assert 10 "tests/79.c"
assert  3 "tests/80.c"
assert  2 "tests/81.c"
assert 10 "tests/82.c"
assert 97 "tests/83.c"
assert 98 "tests/84.c"
assert 99 "tests/85.c"
assert  0 "tests/86.c"
assert  4 "tests/87.c"



echo OK