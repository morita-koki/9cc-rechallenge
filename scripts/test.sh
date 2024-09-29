#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
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

assert 0 "0;"
assert 41 "12 + 34 - 5; "
assert 6 "1  + 2 + 3;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 10 '- -10;'
assert 10 '- - +10;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'
assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'
assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 3 'a=3;'
assert 3 'a=3; a;'
assert 7 'a=3; z=5; c=7;'
assert 8 'a=3; z=5; a+z;'
assert 8 'a=3; z=5; b=a+z;'

assert 5 "foo = 5;"

assert 8 "
bar = 8; 
bar;
"

assert 10 "
foo = 4;
bar = 6;
foo + bar;
"

assert 4 "return 4;"
assert 5 "foo = 5; foo;"
assert 8 "
bar = 8; 
return bar;
"

assert 10 "
bar = 8; 
return bar + 2;
"

assert 10 "
foo = 4;
bar = 6;
return foo + bar;
"



# error check
# assert 6 "1 + 2 ++ 3"
# assert 8 "1 + 2 + 3 3"

echo OK