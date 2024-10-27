int a[4];
int main() {
  init_a();
  return *(a + 3);
}

int init_a() {
  a[0] = 1;
  a[1] = 3;
  a[2] = 5;
  a[3] = 10;
}