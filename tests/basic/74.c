int a[4];
int main() {
  *a = 1;
  *(a + 1) = 3;
  *(a + 2) = 5;
  *(a + 3) = 10;
  return a[3];
}