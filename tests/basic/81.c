int main() {
  char x[4];
  x[0] = 1;
  x[1] = 3;
  x[2] = 5;
  x[3] = 10;
  return char_sub(*(x + 3), char_add(x[2], *(x + 1)));
}
int char_add(char a, char b) { return a + b; }
int char_sub(char a, char b) { return a - b; }