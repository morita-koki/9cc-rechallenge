int main() {
  char x[4];
  x[0] = 1;
  x[1] = 2;
  x[2] = 3;
  x[3] = 4;
  return *(x + 3) + 6;
}