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