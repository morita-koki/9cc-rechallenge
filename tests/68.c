int main() {
  int a[2];
  *a = 1;
  a[1] = 2;
  int *p;
  p = a;
  return a[0] + *(p + 1);
}