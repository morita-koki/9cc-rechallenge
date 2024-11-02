int main() { return func(4); }
int func(int x) {
  if (x == 1) return 1;
  return func(x - 1) + x;
}