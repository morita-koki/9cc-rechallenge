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

type=$1
if [ "$type" = "basic" ]; then
  assert  0 "tests/basic/00.c"
  assert 41 "tests/basic/01.c"
  assert  6 "tests/basic/02.c"
  assert 47 "tests/basic/03.c"
  assert 15 "tests/basic/04.c"
  assert  4 "tests/basic/05.c"
  assert 10 "tests/basic/06.c" 
  assert 10 "tests/basic/07.c" 
  assert 10 "tests/basic/08.c" 
  assert  0 "tests/basic/09.c" 
  assert  1 "tests/basic/10.c" 
  assert  1 "tests/basic/11.c" 
  assert  0 "tests/basic/12.c"  
  assert  1 "tests/basic/13.c"  
  assert  0 "tests/basic/14.c"  
  assert  0 "tests/basic/15.c"  
  assert  1 "tests/basic/16.c"  
  assert  1 "tests/basic/17.c"  
  assert  0 "tests/basic/18.c"  
  assert  1 "tests/basic/19.c"  
  assert  0 "tests/basic/20.c"  
  assert  0 "tests/basic/21.c"  
  assert  1 "tests/basic/22.c"  
  assert  1 "tests/basic/23.c"  
  assert  0 "tests/basic/24.c"
  assert  3 "tests/basic/25.c"
  assert  3 "tests/basic/26.c"
  assert  8 "tests/basic/27.c"
  assert  8 "tests/basic/28.c"
  assert 10 "tests/basic/29.c"
  assert 10 "tests/basic/30.c"
  assert 10 "tests/basic/31.c"
  assert 10 "tests/basic/32.c"
  assert 10 "tests/basic/33.c"
  assert 10 "tests/basic/34.c"
  assert 10 "tests/basic/35.c"
  assert 10 "tests/basic/36.c"
  assert 10 "tests/basic/37.c"
  assert 10 "tests/basic/38.c"
  assert 10 "tests/basic/39.c"
  assert 10 "tests/basic/40.c"
  assert  5 "tests/basic/41.c"
  assert 10 "tests/basic/42.c"
  assert 10 "tests/basic/43.c"
  assert 10 "tests/basic/44.c"
  assert 10 "tests/basic/45.c"
  assert 10 "tests/basic/46.c"
  assert 10 "tests/basic/47.c"
  assert 21 "tests/basic/48.c"
  assert 10 "tests/basic/49.c"
  assert 10 "tests/basic/50.c"
  assert 10 "tests/basic/51.c"
  assert 10 "tests/basic/52.c"
  assert 10 "tests/basic/53.c"
  assert 10 "tests/basic/54.c"
  assert 10 "tests/basic/55.c"
  assert 10 "tests/basic/56.c"
  assert 10 "tests/basic/57.c"
  assert 10 "tests/basic/58.c"
  assert  5 "tests/basic/59.c"
  assert  3 "tests/basic/60.c"
  assert 10 "tests/basic/61.c"
  assert  4 "tests/basic/62.c"
  assert  8 "tests/basic/63.c"
  assert  8 "tests/basic/64.c"
  assert 40 "tests/basic/65.c"
  assert 10 "tests/basic/66.c"
  assert  3 "tests/basic/67.c"
  assert  3 "tests/basic/68.c"
  assert 10 "tests/basic/69.c"
  assert 10 "tests/basic/70.c"
  assert 10 "tests/basic/71.c"
  assert 10 "tests/basic/72.c"
  assert 10 "tests/basic/73.c"
  assert 10 "tests/basic/74.c"
  assert 10 "tests/basic/75.c"
  assert 10 "tests/basic/76.c"
  assert 10 "tests/basic/77.c"
  assert 10 "tests/basic/78.c"
  assert 10 "tests/basic/79.c"
  assert  3 "tests/basic/80.c"
  assert  2 "tests/basic/81.c"
  assert 10 "tests/basic/82.c"
  assert 97 "tests/basic/83.c"
  assert 98 "tests/basic/84.c"
  assert 99 "tests/basic/85.c"
  assert  0 "tests/basic/86.c"
  assert  4 "tests/basic/87.c"
fi


if [ "$type" = "advance" ]; then
  assert 10 "tests/advance/00.c"
  assert 10 "tests/advance/01.c"
  assert 10 "tests/advance/02.c"
  assert  3 "tests/advance/03.c"
  assert  0 "tests/advance/04.c"
  assert  3 "tests/advance/05.c"
  assert  0 "tests/advance/06.c"
  assert  6 "tests/advance/07.c"
  assert  6 "tests/advance/08.c"
  assert 24 "tests/advance/09.c"
  assert 24 "tests/advance/10.c"
  assert 24 "tests/advance/11.c"
  assert 67 "tests/advance/12.c"
  assert 67 "tests/advance/13.c"
  assert 67 "tests/advance/14.c"
  assert 67 "tests/advance/15.c"
fi

echo OK